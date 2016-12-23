//
// Created by fred on 12/12/16.
//

#include "frnetlib/SSLSocket.h"
#include <memory>
#ifdef SSL_ENABLED

namespace fr
{
    SSLSocket::SSLSocket(std::shared_ptr<SSLContext> ssl_context_) noexcept
    : recv_buffer(new char[RECV_CHUNK_SIZE]),
      ssl_context(ssl_context_)
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
        if(is_connected)
        {
            if(ssl)
                mbedtls_ssl_close_notify(ssl.get());
            if(ssl_socket_descriptor)
                mbedtls_net_free(ssl_socket_descriptor.get());
            is_connected = false;
        }
    }

    Socket::Status SSLSocket::send_raw(const char *data, size_t size)
    {
        std::lock_guard<std::mutex> guard(outbound_mutex);
        int error = 0;
        while((error = mbedtls_ssl_write(ssl.get(), (const unsigned char *)data, size)) <= 0)
        {
            if(error != MBEDTLS_ERR_SSL_WANT_READ && error != MBEDTLS_ERR_SSL_WANT_WRITE)
            {
                return Socket::Status::Error;
            }
        }

        return Socket::Status::Success;
    }

    Socket::Status SSLSocket::receive_raw(void *data, size_t data_size, size_t &received)
    {
        std::lock_guard<std::mutex> guard(inbound_mutex);

        int read = MBEDTLS_ERR_SSL_WANT_READ;
        received = 0;
        if(unprocessed_buffer.size() < data_size)
        {
            while(read == MBEDTLS_ERR_SSL_WANT_READ || read == MBEDTLS_ERR_SSL_WANT_WRITE)
            {
                read = mbedtls_ssl_read(ssl.get(), (unsigned char *)recv_buffer.get(), RECV_CHUNK_SIZE);
            }

            if(read == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY)
            {
                is_connected = false;
                return Socket::Status::Disconnected;
            }
            else if(read <= 0)
            {
                //No data. But no error occurred.
                return Socket::Status::Success;
            }

            received += read;
            unprocessed_buffer += {recv_buffer.get(), (size_t)read};

            if(received > data_size)
                received = data_size;
        }
        else
        {
            received = data_size;
        }

        //Copy data to where it needs to go
        memcpy(data, &unprocessed_buffer[0], received);
        unprocessed_buffer.erase(0, received);
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

        //Update members
        is_connected = true;
        remote_address = address + ":" + port;
        return Socket::Status::Success;
    }

    void SSLSocket::set_ssl_context(std::unique_ptr<mbedtls_ssl_context> context)
    {
        ssl = std::move(context);
    }

    void SSLSocket::set_net_context(std::unique_ptr<mbedtls_net_context> context)
    {
        is_connected = true;
        ssl_socket_descriptor = std::move(context);
    }
}

#endif