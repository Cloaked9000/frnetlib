//
// Created by fred on 12/12/16.
//

#include "frnetlib/SSLSocket.h"
#include <memory>
#include <utility>

#include <mbedtls/net_sockets.h>
#include <frnetlib/SSLSocket.h>


namespace fr
{
    SSLSocket::SSLSocket(std::shared_ptr<SSLContext> ssl_context_) noexcept
    :  ssl_context(std::move(ssl_context_)),
       should_verify(true),
       receive_timeout(0)
    {
        //Initialise mbedtls structures
        mbedtls_ssl_config_init(&conf);
    }

    SSLSocket::~SSLSocket() noexcept
    {
        //Close connection if active
        close_socket();

        //Cleanup mbedtls stuff
        mbedtls_ssl_config_free(&conf);
    }

    void SSLSocket::close_socket()
    {
        if(ssl)
        {
            mbedtls_ssl_close_notify(ssl.get());
            mbedtls_ssl_free(ssl.get());
        }

        if(ssl_socket_descriptor)
            mbedtls_net_free(ssl_socket_descriptor.get());
        ssl_socket_descriptor = nullptr;
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
                return Socket::Status::Error;
            }
        }

        return Socket::Status::Success;
    }

    Socket::Status SSLSocket::receive_raw(void *data, size_t data_size, size_t &received)
    {
        ssize_t status = 0;
        if(receive_timeout == 0)
        {
            status = mbedtls_ssl_read(ssl.get(), (unsigned char *)data, data_size);
            if(status <= 0)
            {
                if(status == MBEDTLS_ERR_SSL_WANT_READ || status == MBEDTLS_ERR_SSL_WANT_WRITE)
                {
                    return Socket::Status::WouldBlock;
                }

                return Socket::Status::Error;
            }
        }
        else
        {
            do
            {
                status = mbedtls_net_recv_timeout(ssl.get(), (unsigned char *)data, data_size, receive_timeout);
                if(status <= 0)
                {
                    if(status == MBEDTLS_ERR_SSL_TIMEOUT)
                    {
                        return Socket::Status::WouldBlock;
                    }
                    else if(status == MBEDTLS_ERR_SSL_WANT_READ)
                    {
                        continue; //try again, interrupted before anything could be received
                    }

                    return Socket::Status::Error;
                }
                break;
            } while(true);
        }


        received = static_cast<size_t>(status);
        return Socket::Status::Success;

    }

    Socket::Status SSLSocket::connect(const std::string &address, const std::string &port, std::chrono::seconds timeout)
    {
        //Initialise mbedtls stuff
        ssl = std::make_unique<mbedtls_ssl_context>();
        ssl_socket_descriptor = std::make_unique<mbedtls_net_context>();
        mbedtls_ssl_init(ssl.get());
        mbedtls_net_init(ssl_socket_descriptor.get());

        //Due to mbedtls not supporting connect timeouts, we have to use an fr::TcpSocket to
        //Open the descriptor, and then steal it. This is a hack.
        {
            fr::TcpSocket socket;
            auto ret = socket.connect(address, port, timeout);
            if(ret != fr::Socket::Success)
                return ret;
            ssl_socket_descriptor->fd = socket.get_socket_descriptor();
            remote_address = socket.get_remote_address();
            socket.set_descriptor(nullptr);
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
                return Socket::Status::HandshakeFailed;
            }
        }

        //Verify server certificate
        if(should_verify && ((flags = mbedtls_ssl_get_verify_result(ssl.get())) != 0))
        {
            char verify_buffer[512];
            mbedtls_x509_crt_verify_info(verify_buffer, sizeof(verify_buffer), "  ! ", flags);

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

    void SSLSocket::set_descriptor(void *descriptor)
    {
        ssl_socket_descriptor.reset(static_cast<mbedtls_net_context*>(descriptor));
        if(descriptor)
            reconfigure_socket();
    }

    void SSLSocket::verify_certificates(bool should_verify_)
    {
        should_verify = should_verify_;
    }

    void SSLSocket::reconfigure_socket()
    {
        if(!connected())
        {
            return;
        }

        int one = 1;
#ifndef _WIN32
        //Disable Nagle's algorithm
        setsockopt(get_socket_descriptor(), SOL_TCP, TCP_NODELAY, (char*)&one, sizeof(one));
#else
        //Disable Nagle's algorithm
        setsockopt(get_socket_descriptor(), IPPROTO_TCP, TCP_NODELAY, (char*)&one, sizeof(one));
        setsockopt(get_socket_descriptor(), SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char*)&one, sizeof(one));

        //Apply receive timeout
        DWORD timeout_dword = static_cast<DWORD>(get_receive_timeout());
        setsockopt(get_socket_descriptor(), SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout_dword, sizeof timeout_dword);
#endif
    }
}
