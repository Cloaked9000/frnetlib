//
// Created by fred on 12/12/16.
//

#include "frnetlib/SSLSocket.h"
#include <memory>
#include <mbedtls/net_sockets.h>

#ifdef SSL_ENABLED

namespace fr
{
    SSLSocket::SSLSocket(std::shared_ptr<SSLContext> ssl_context_) noexcept
    :  ssl_context(ssl_context_)
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
        if(ssl_socket_descriptor->fd > -1)
        {
            if(ssl)
                mbedtls_ssl_close_notify(ssl.get());
            if(ssl_socket_descriptor)
                mbedtls_net_free(ssl_socket_descriptor.get());
        }
    }

    Socket::Status SSLSocket::send_raw(const char *data, size_t size)
    {
        int error = 0;
        while((error = mbedtls_ssl_write(ssl.get(), (const unsigned char *)data, size)) <= 0)
        {
            if(error != MBEDTLS_ERR_SSL_WANT_READ && error != MBEDTLS_ERR_SSL_WANT_WRITE)
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

        if(read == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY)
        {
            close_socket();
            return Socket::Status::Disconnected;
        }
        else if(read <= 0)
        {
            //No data. But no error occurred.
            return Socket::Status::Success;
        }

        received += read;
        return Socket::Status::Success;

    }

    Socket::Status SSLSocket::connect(const std::string &address, const std::string &port)
    {
        //Initialise mbedtls stuff
        ssl = std::unique_ptr<mbedtls_ssl_context>(new mbedtls_ssl_context);
        ssl_socket_descriptor = std::unique_ptr<mbedtls_net_context>(new mbedtls_net_context);
        mbedtls_ssl_init(ssl.get());
        mbedtls_net_init(ssl_socket_descriptor.get());

        //Initialise the connection using mbedtlsl
        int error = 0;
        if((error = mbedtls_net_connect(ssl_socket_descriptor.get(), address.c_str(), port.c_str(), MBEDTLS_NET_PROTO_TCP)) != 0)
        {
            return Socket::Status::ConnectionFailed;
        }

        //Initialise SSL data structures
        if((error = mbedtls_ssl_config_defaults(&conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
        {
            return Socket::Status::Error;
        }

        mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_REQUIRED);
        mbedtls_ssl_conf_ca_chain(&conf, &ssl_context->cacert, NULL);
        mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ssl_context->ctr_drbg);

        if((error = mbedtls_ssl_setup(ssl.get(), &conf)) != 0)
        {
            return Socket::Status::Error;
        }

        if((error = mbedtls_ssl_set_hostname(ssl.get(), address.c_str())) != 0)
        {
            return Socket::Status::Error;
        }

        mbedtls_ssl_set_bio(ssl.get(), ssl_socket_descriptor.get(), mbedtls_net_send, mbedtls_net_recv, NULL);

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
        if((flags = mbedtls_ssl_get_verify_result(ssl.get())) != 0)
        {
            char verify_buffer[512];
            mbedtls_x509_crt_verify_info( verify_buffer, sizeof( verify_buffer ), "  ! ", flags );

            std::cout << "Failed to connect to server. Server certificate validation failed: " << verify_buffer << std::endl;
            return Socket::Status::VerificationFailed;
        }

        //Update state
        remote_address = address + ":" + port;
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
}

#endif