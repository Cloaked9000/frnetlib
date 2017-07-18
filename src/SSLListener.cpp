//
// Created by fred on 13/12/16.
//

#include <chrono>
#include <frnetlib/TcpListener.h>
#include "frnetlib/SSLListener.h"
#ifdef SSL_ENABLED

#include <mbedtls/net_sockets.h>

namespace fr
{
    SSLListener::SSLListener(std::shared_ptr<SSLContext> ssl_context_, const std::string &crt_path, const std::string &pem_path, const std::string &private_key_path) noexcept
    : ssl_context(ssl_context_)
    {
        //Initialise SSL objects required
        listen_fd.fd = -1;
        mbedtls_ssl_config_init(&conf);
        mbedtls_x509_crt_init(&srvcert);
        mbedtls_pk_init(&pkey);

        int error = 0;

        //Load certificates and private key
        error = mbedtls_x509_crt_parse_file(&srvcert, crt_path.c_str());
        if(error != 0)
        {
            std::cout << "Failed to initialise SSL listener. CRT Parse returned: " << error << std::endl;
            return;
        }

        error = mbedtls_x509_crt_parse_file(&srvcert, pem_path.c_str());
        if(error != 0)
        {
            std::cout << "Failed to initialise SSL listener. PEM Parse returned: " << error << std::endl;
            return;
        }

        error = mbedtls_pk_parse_keyfile(&pkey, private_key_path.c_str(), 0);
        if(error != 0)
        {
            std::cout << "Failed to initialise SSL listener. Private Key Parse returned: " << error << std::endl;
            return;
        }

        //Setup data structures
        if((error = mbedtls_ssl_config_defaults(&conf, MBEDTLS_SSL_IS_SERVER, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
        {
            std::cout << "Failed to configure SSL presets: " << error << std::endl;
            return;
        }

        //Apply them
        mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ssl_context->ctr_drbg);
        mbedtls_ssl_conf_ca_chain(&conf, srvcert.next, NULL);

        if((error = mbedtls_ssl_conf_own_cert(&conf, &srvcert, &pkey)) != 0)
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
        SSLSocket &client = dynamic_cast<SSLSocket&>(client_);

        //Initialise mbedtls
        int error = 0;
        std::unique_ptr<mbedtls_ssl_context> ssl(new mbedtls_ssl_context);
        mbedtls_ssl_init(ssl.get());
        if((error = mbedtls_ssl_setup(ssl.get(), &conf ) ) != 0)
        {
            std::cout << "Failed to apply SSL setings: " << error << std::endl;
            return Socket::Error;
        }

        //Accept a connection
        std::unique_ptr<mbedtls_net_context> client_fd(new mbedtls_net_context);
        mbedtls_net_init(client_fd.get());

        if((error = mbedtls_net_accept(&listen_fd, client_fd.get(), NULL, 0, NULL)) != 0)
        {
            std::cout << "Accept error: " << error << std::endl;
            return Socket::Error;
        }

        mbedtls_ssl_set_bio(ssl.get(), client_fd.get(), mbedtls_net_send, mbedtls_net_recv, NULL);

        //SSL Handshake
        while((error = mbedtls_ssl_handshake(ssl.get())) != 0)
        {
            if(error != MBEDTLS_ERR_SSL_WANT_READ && error != MBEDTLS_ERR_SSL_WANT_WRITE)
            {
                return Socket::Status::HandshakeFailed;
            }
        }

        //Set socket details
        client.set_net_context(std::move(client_fd));
        client.set_ssl_context(std::move(ssl));
        return Socket::Success;
    }

    void SSLListener::shutdown()
    {
        ::shutdown(listen_fd.fd, 0);
    }

    int32_t SSLListener::get_socket_descriptor() const
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

}
#endif