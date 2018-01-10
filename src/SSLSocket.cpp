//
// Created by fred on 12/12/16.
//

#include "frnetlib/SSLSocket.h"
#include <memory>
#include <utility>

#ifdef SSL_ENABLED

#include <mbedtls/net_sockets.h>

namespace fr
{
    SSLSocket::SSLSocket(std::shared_ptr<SSLContext> ssl_context_) noexcept
    :  ssl_context(std::move(ssl_context_)),
       should_verify(true)
    {
        //Initialise mbedtls structures
        mbedtls_ssl_config_init(&conf);

    }

    SSLSocket::~SSLSocket() noexcept
    {
        //Close connection if active
        close_socket();

        //Cleanup mbedsql stuff
        mbedtls_ssl_config_free(&conf);
    }

    void SSLSocket::close_socket()
    {
        if(ssl_socket_descriptor)
            mbedtls_net_free(ssl_socket_descriptor.get());
        if(ssl)
        {
            mbedtls_ssl_close_notify(ssl.get());
            mbedtls_ssl_free(ssl.get());
        }

    }

    Socket::Status SSLSocket::send_raw(const char *data, size_t size)
    {
        int response = 0;
        size_t data_sent = 0;
        while(data_sent < size)
        {
            response = mbedtls_ssl_write(ssl.get(), (const unsigned char *)data + data_sent, size - data_sent);
            if(response != MBEDTLS_ERR_SSL_WANT_READ && response != MBEDTLS_ERR_SSL_WANT_WRITE)
            {
                data_sent += response;
            }
            else if(response < 0)
            {
                close_socket();
                return Socket::Status::Disconnected;
            }
        }

        return Socket::Status::Success;
    }

    Socket::Status SSLSocket::receive_raw(void *data, size_t data_size, size_t &received)
    {
        int read = MBEDTLS_ERR_SSL_WANT_READ;
        received = 0;

        while(read == MBEDTLS_ERR_SSL_WANT_READ || read == MBEDTLS_ERR_SSL_WANT_WRITE)
        {
            read = mbedtls_ssl_read(ssl.get(), (unsigned char *)data, data_size);
        }

        if(read <= 0)
        {
            close_socket();
            return Socket::Status::Disconnected;
        }

        received += read;
        return Socket::Status::Success;

    }

    Socket::Status SSLSocket::connect(const std::string &address, const std::string &port, std::chrono::seconds timeout)
    {
        //Initialise mbedtls stuff
        ssl = std::make_unique<mbedtls_ssl_context>();
        ssl_socket_descriptor = std::make_unique<mbedtls_net_context>();
        mbedtls_ssl_init(ssl.get());
        mbedtls_net_init(ssl_socket_descriptor.get());

        //Do to mbedtls not supporting connect timeouts, we have to use an fr::TcpSocket to
        //Open the descriptor, and then steal it. This is a hack.
        {
            fr::TcpSocket socket;
            auto ret = socket.connect(address, port, timeout);
            if(ret != fr::Socket::Success)
                return ret;
            ssl_socket_descriptor->fd = socket.get_socket_descriptor();
            remote_address = socket.get_remote_address();
            socket.set_descriptor(-1);
        }

        //Initialise SSL data structures
        int error = 0;
        if((error = mbedtls_ssl_config_defaults(&conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
        {
            return Socket::Status::Error;
        }

        mbedtls_ssl_conf_authmode(&conf, should_verify ? MBEDTLS_SSL_VERIFY_REQUIRED : MBEDTLS_SSL_VERIFY_NONE);
        mbedtls_ssl_conf_ca_chain(&conf, &ssl_context->cacert, nullptr);
        mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ssl_context->ctr_drbg);

        if((error = mbedtls_ssl_setup(ssl.get(), &conf)) != 0)
        {
            return Socket::Status::Error;
        }

        if((error = mbedtls_ssl_set_hostname(ssl.get(), address.c_str())) != 0)
        {
            return Socket::Status::Error;
        }

        mbedtls_ssl_set_bio(ssl.get(), ssl_socket_descriptor.get(), mbedtls_net_send, mbedtls_net_recv, nullptr);

        //Do SSL handshake
        while((error = mbedtls_ssl_handshake(ssl.get())) != 0)
        {
            if(error != MBEDTLS_ERR_SSL_WANT_READ && error != MBEDTLS_ERR_SSL_WANT_WRITE)
            {
                std::cout << "Failed to connect to server. Handshake returned: " << error << std::endl;
                return Socket::Status::HandshakeFailed;
            }
        }

        //Verify server certificate
        if(should_verify && ((flags = mbedtls_ssl_get_verify_result(ssl.get())) != 0))
        {
            char verify_buffer[512];
            mbedtls_x509_crt_verify_info( verify_buffer, sizeof( verify_buffer ), "  ! ", flags );

            std::cout << "Failed to connect to server. Server certificate validation failed: " << verify_buffer << std::endl;
            return Socket::Status::VerificationFailed;
        }

        //Update state
        reconfigure_socket();

        return Socket::Status::Success;
    }

    void SSLSocket::set_ssl_context(std::unique_ptr<mbedtls_ssl_context> context)
    {
        ssl = std::move(context);
    }

    void SSLSocket::set_net_context(std::unique_ptr<mbedtls_net_context> context)
    {
        ssl_socket_descriptor = std::move(context);
        reconfigure_socket();
    }

    void SSLSocket::verify_certificates(bool should_verify_)
    {
        should_verify = should_verify_;
    }
}

#endif