//
// Created by fred on 13/12/16.
//

#include <chrono>
#include <utility>
#include "frnetlib/NetworkEncoding.h"
#include "frnetlib/TcpListener.h"
#include "frnetlib/SSLListener.h"

#include <mbedtls/net_sockets.h>
#include <iostream>
#include <frnetlib/SSLListener.h>


namespace fr
{
    SSLListener::SSLListener(std::shared_ptr<SSLContext> ssl_context_, const std::string &pem_path, const std::string &private_key_path)
    : ssl_context(std::move(ssl_context_))
    {
        //Initialise SSL objects required
        listen_fd.fd = -1;
        mbedtls_ssl_config_init(&conf);
        mbedtls_x509_crt_init(&srvcert);
        mbedtls_pk_init(&pkey);

        int error = 0;

        //Load public key
        error = mbedtls_x509_crt_parse_file(&srvcert, pem_path.c_str());
        if(error != 0)
        {
            throw std::runtime_error("Error parsing '" + pem_path + "': mbedtls_x509_crt_parse_file() returned " + std::to_string(error));
        }

        //Load private key
        error = mbedtls_pk_parse_keyfile(&pkey, private_key_path.c_str(), 0);
        if(error != 0)
        {
            throw std::runtime_error("Error parsing '" + private_key_path + "': mbedtls_pk_parse_keyfile() returned " + std::to_string(error));
        }

        //Setup data structures and apply settings
        error = mbedtls_ssl_config_defaults(&conf, MBEDTLS_SSL_IS_SERVER, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT);
        if(error != 0)
        {
            throw std::runtime_error("mbedtls_ssl_config_defaults() returned: " + std::to_string(error));
        }
        mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ssl_context->ctr_drbg);
        mbedtls_ssl_conf_ca_chain(&conf, srvcert.next, nullptr);

        //Apply loaded certs
        error = mbedtls_ssl_conf_own_cert(&conf, &srvcert, &pkey);
        if(error != 0)
        {
            std::cout << "Failed to set certificate: " << error << std::endl;
            return;
        }
    }

    SSLListener::~SSLListener()
    {
        close_socket();
        mbedtls_x509_crt_free(&srvcert);
        mbedtls_pk_free(&pkey);
        mbedtls_ssl_config_free(&conf);
        mbedtls_net_free(&listen_fd);
    }

    Socket::Status fr::SSLListener::listen(const std::string &port)
    {
        //This is a hack. mbedtls doesn't support specifying the address family.
        close_socket();
        mbedtls_net_init(&listen_fd);
        fr::TcpListener tcp_listen;
        tcp_listen.set_inet_version(ai_family);
        if(tcp_listen.listen(port) != fr::Socket::Success)
        {
            return Socket::BindFailed;
        }

        listen_fd.fd = tcp_listen.get_socket_descriptor();
        tcp_listen.set_socket_descriptor(-1); //The socket wont close if it's -1 when we destruct it
        return Socket::Success;
    }

    Socket::Status SSLListener::accept(Socket &client_)
    {
        //Cast to SSLSocket. Will throw bad cast on failure.
        auto &client = dynamic_cast<SSLSocket&>(client_);

        //Initialise mbedtls
        int error = 0;
        auto ssl = std::make_unique<mbedtls_ssl_context>();
        auto client_fd = std::make_unique<mbedtls_net_context>();

        mbedtls_ssl_init(ssl.get());
        mbedtls_net_init(client_fd.get());
        auto free_contexts = [&](){mbedtls_ssl_free(ssl.get()); mbedtls_net_free(client_fd.get());};
        if((error = mbedtls_ssl_setup(ssl.get(), &conf)) != 0)
        {
            free_contexts();
            return Socket::Error;
        }

        //Accept a connection
        char client_ip[INET6_ADDRSTRLEN] = {0};
        size_t ip_len = 0;
        if((error = mbedtls_net_accept(&listen_fd, client_fd.get(), client_ip, sizeof(client_ip), &ip_len)) != 0)
        {
            free_contexts();
            return Socket::Error;
        }


        //SSL Handshake
        mbedtls_ssl_set_bio(ssl.get(), client_fd.get(), mbedtls_net_send, mbedtls_net_recv, nullptr);
        while((error = mbedtls_ssl_handshake(ssl.get())) != 0)
        {
            if(error != MBEDTLS_ERR_SSL_WANT_READ && error != MBEDTLS_ERR_SSL_WANT_WRITE)
            {
                free_contexts();
                return Socket::Status::HandshakeFailed;
            }
        }

        //Get remote address and port. We could get the IP from the accept args, but we also want the port
        //Which mbedtls doesn't provide
        //Get printable address. If we failed then set it as just 'unknown'
        char client_printable_addr[INET6_ADDRSTRLEN];
        struct sockaddr_storage socket_address{};
        socklen_t socket_length = sizeof(socket_address);
        error = getpeername(client_fd->fd, (struct sockaddr*)&socket_address, &socket_length);
        if(error == 0)
        {
            error = getnameinfo((sockaddr*)&socket_address, socket_length, client_printable_addr, sizeof(client_printable_addr), nullptr,0,NI_NUMERICHOST);
        }
        if(error != 0)
        {
            strcpy(client_printable_addr, "unknown");
        }

        client.set_ssl_context(std::move(ssl));
        client.set_descriptor(client_fd.release());
        client.set_remote_address(client_printable_addr);
        return Socket::Success;
    }

    void SSLListener::shutdown()
    {
        ::shutdown(listen_fd.fd, 0);
    }

    int32_t SSLListener::get_socket_descriptor() const noexcept
    {
        return listen_fd.fd;
    }

    void SSLListener::set_socket_descriptor(int32_t descriptor)
    {
        listen_fd.fd = descriptor;
    }

    void SSLListener::close_socket()
    {
        if(listen_fd.fd != -1)
        {
            mbedtls_net_free(&listen_fd);
            listen_fd.fd = -1;
        }
    }

    bool SSLListener::connected() const noexcept
    {
        return listen_fd.fd > -1;
    }

}