//
// Created by fred on 12/12/16.
//

#ifndef FRNETLIB_SSL_SOCKET_H
#define FRNETLIB_SSL_SOCKET_H

#ifdef SSL_ENABLED

#include "TcpSocket.h"
#include "SSLContext.h"
#include <mbedtls/net_sockets.h>
#include <mbedtls/debug.h>
#include <mbedtls/ssl.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/error.h>
#include <mbedtls/certs.h>

namespace fr
{
    class SSLSocket : public Socket
    {
    public:
        SSLSocket(std::shared_ptr<SSLContext> ssl_context) noexcept;

        ~SSLSocket() noexcept;

        SSLSocket(SSLSocket &&) noexcept = default;

        /*!
         * Effectively just fr::TcpSocket::send_raw() with encryption
         * added in.
         *
         * @param data The data to send.
         * @param size The number of bytes, from data to send. Be careful not to overflow.
         * @return The status of the operation.
         */
        Socket::Status send_raw(const char *data, size_t size) override;


        /*!
         * Effectively just fr::TcpSocket::receive_raw() with encryption
         * added in.
         *
         * @param data Where to store the received data.
         * @param data_size The number of bytes to try and receive. Be sure that it's not larger than data.
         * @param received Will be filled with the number of bytes actually received, might be less than you requested.
         * @return The status of the operation, if the socket has disconnected etc.
         */
        Socket::Status receive_raw(void *data, size_t data_size, size_t &received) override;

        /*!
         * Close the connection.
         */
        void close_socket() override;

        /*!
         * Connects the socket to an address.
         *
         * @param address The address of the socket to connect to
         * @param port The port of the socket to connect to
         * @return A Socket::Status indicating the status of the operation.
         */
        Socket::Status connect(const std::string &address, const std::string &port) override;

        /*!
         * Set the SSL context
         *
         * @param context The SSL context to use
         */
        void set_ssl_context(std::unique_ptr<mbedtls_ssl_context> context);

        /*!
         * Set the NET context
         *
         * @param context The NET context to use
         */
        void set_net_context(std::unique_ptr<mbedtls_net_context> context);

        /*!
         * Gets the underlying socket descriptor.
         *
         * @return The socket's descriptor.
         */
        virtual int32_t get_socket_descriptor() const override
        {
            return ssl_socket_descriptor->fd;
        }

        /*!
         * Sets if the socket should block or not.
         *
         * @param should_block True to block, false otherwise.
         */
        virtual void set_blocking(bool should_block) override
        {
            abort();
        }

    private:
        std::string unprocessed_buffer;
        std::unique_ptr<char[]> recv_buffer;
        std::shared_ptr<SSLContext> ssl_context;

        std::unique_ptr<mbedtls_net_context> ssl_socket_descriptor;
        std::unique_ptr<mbedtls_ssl_context> ssl;
        mbedtls_ssl_config conf;
        uint32_t flags;

        std::mutex outbound_mutex;
        std::mutex inbound_mutex;
    };
}

#endif

#endif //FRNETLIB_SSLSOCKET_H
