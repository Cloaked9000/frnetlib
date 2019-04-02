//
// Created by fred on 12/12/16.
//

#include "frnetlib/SSLSocket.h"
#include <memory>
#include <utility>

#include <mbedtls/net_sockets.h>
#include <frnetlib/SSLSocket.h>

mbedtls_net_context *net_create()
{
    auto *ctx = new mbedtls_net_context;
    mbedtls_net_init(ctx);
    return ctx;
}

mbedtls_ssl_context *ssl_create()
{
    auto *ctx = new mbedtls_ssl_context;
    mbedtls_ssl_init(ctx);
    return ctx;
}

void ssl_free(mbedtls_ssl_context *ctx)
{
    mbedtls_ssl_free(ctx);
    delete ctx;
}

void web_free(mbedtls_net_context *ctx)
{
    mbedtls_net_free(ctx);
    delete ctx;
}

namespace fr
{
    SSLSocket::SSLSocket(std::shared_ptr<SSLContext> ssl_context_) noexcept
    :  ssl_context(std::move(ssl_context_)),
       ssl_socket_descriptor(nullptr, web_free),
       ssl(nullptr, ssl_free),
       should_verify(true),
       receive_timeout(0),
       is_blocking(true)
    {
        //Initialise mbedtls structures
        mbedtls_ssl_config_init(&conf);
    }

    SSLSocket::~SSLSocket()
    {
        //Close connection if active
        close_socket();

        //Cleanup mbedtls stuff
        mbedtls_ssl_config_free(&conf);
    }

    void SSLSocket::close_socket()
    {
        ssl = nullptr;
        ssl_socket_descriptor = nullptr;
    }

    Socket::Status SSLSocket::send_raw(const char *data, size_t size, size_t &sent)
    {
        sent = 0;
        ssize_t status = 0;

        while(sent < size)
        {
            status = mbedtls_ssl_write(ssl.get(), (const unsigned char *)data + sent, size - sent);
            if(status < 0)
            {
                if(status == MBEDTLS_ERR_SSL_WANT_READ || status == MBEDTLS_ERR_SSL_WANT_WRITE)
                {
                    if(is_blocking)
                    {
                        return Socket::Status::Timeout;
                    }
                    return Socket::Status::WouldBlock;
                }

                errno = status;
                return Socket::Status::SSLError;
            }

            sent += status;
        }

        return Socket::Status::Success;
    }

    Socket::Status SSLSocket::receive_raw(void *data, size_t data_size, size_t &received)
    {
        ssize_t status = 0;
        if(receive_timeout == 0)
        {
            do
            {
                status = mbedtls_ssl_read(ssl.get(), (unsigned char *) data, data_size);
                if(status == 0)
                {
                    return Socket::Status::Disconnected;
                }
                if(status < 0)
                {
                    if(status == MBEDTLS_ERR_SSL_WANT_READ || status == MBEDTLS_ERR_SSL_WANT_WRITE)
                    {
                        if(is_blocking)
                        {
                            return Socket::Status::Timeout;
                        }
                        continue;
                    }

                    errno = static_cast<int>(status);
                    return Socket::Status::SSLError;
                }
                break;
            } while(true);
        }
        else
        {
            do
            {
                status = mbedtls_net_recv_timeout(ssl.get(), (unsigned char *)data, data_size, receive_timeout);
                if(status == 0)
                {
                    return Socket::Status::Disconnected;
                }

                if(status < 0)
                {
                    if(status == MBEDTLS_ERR_SSL_TIMEOUT)
                    {
                        return Socket::Status::WouldBlock;
                    }

                    if(status == MBEDTLS_ERR_SSL_WANT_READ)
                    {
                        continue; //try again, interrupted before anything could be received
                    }

                    errno = static_cast<int>(status);
                    return Socket::Status::SSLError;
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
        ssl.reset(ssl_create());
        ssl_socket_descriptor.reset(net_create());

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
            errno = error;
            return Socket::Status::SSLError;
        }

        mbedtls_ssl_conf_authmode(&conf, should_verify ? MBEDTLS_SSL_VERIFY_REQUIRED : MBEDTLS_SSL_VERIFY_NONE);
        mbedtls_ssl_conf_ca_chain(&conf, &ssl_context->cacert, nullptr);
        mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ssl_context->ctr_drbg);

        if((error = mbedtls_ssl_setup(ssl.get(), &conf)) != 0)
        {
            errno = error;
            return Socket::Status::SSLError;
        }

        if((error = mbedtls_ssl_set_hostname(ssl.get(), address.c_str())) != 0)
        {
            errno = error;
            return Socket::Status::SSLError;
        }

        mbedtls_ssl_set_bio(ssl.get(), ssl_socket_descriptor.get(), mbedtls_net_send, mbedtls_net_recv, nullptr);

        //Do SSL handshake
        while((error = mbedtls_ssl_handshake(ssl.get())) != 0)
        {
            if(error != MBEDTLS_ERR_SSL_WANT_READ && error != MBEDTLS_ERR_SSL_WANT_WRITE)
            {
                errno = error;
                return Socket::Status::SSLError;
            }
        }

        //Verify server certificate
        if(should_verify)
        {
            if(mbedtls_ssl_get_verify_result(ssl.get()) != 0)
            {
                return Socket::Status::VerificationFailed;
            }
        }

        //Update state
        reconfigure_socket();

        return Socket::Status::Success;
    }

    void SSLSocket::set_ssl_context(std::unique_ptr<mbedtls_ssl_context> context)
    {
        ssl.reset(context.release());
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