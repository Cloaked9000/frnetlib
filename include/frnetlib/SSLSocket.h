//
// Created by fred on 12/12/16.
//

#ifndef FRNETLIB_SSL_SOCKET_H
#define FRNETLIB_SSL_SOCKET_H
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
        explicit SSLSocket(std::shared_ptr<SSLContext> ssl_context) noexcept;
        virtual ~SSLSocket() noexcept;
        SSLSocket(SSLSocket &&) = delete;
        SSLSocket(const SSLSocket &) = delete;
        void operator=(SSLSocket &&)=delete;
        void operator=(const SSLSocket &)=delete;

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
         * @return The status of the operation:
         * 'WouldBlock' if no data has been received, and the socket is in non-blocking mode
         * 'Disconnected' if the socket has disconnected.
         * 'Success' All the bytes you wanted have been read
         */
        Socket::Status receive_raw(void *data, size_t data_size, size_t &received) override;

        /*!
         * Connects the socket to an address.
         *
         * @param address The address of the socket to connect to
         * @param port The port of the socket to connect to
         * @param timeout The number of seconds to wait before timing the connection attempt out. Pass -1 for default.
         * @return A Socket::Status indicating the status of the operation.
         */
        Socket::Status connect(const std::string &address, const std::string &port, std::chrono::seconds timeout) override;

        /*!
         * Sets the socket file descriptor. Internally used.
         *
         * @note For SSLSocket, this should be a pointer to a heap allocated mbedtls_net_context. Pointer ownership will be taken over by the SSLSocket.
         * @param descriptor_data The socket descriptor data, set up by the Listener.
         */
        void set_descriptor(void *descriptor_data) override;

        /*!
         * Set the SSL context
         *
         * @param context The SSL context to use
         */
        void set_ssl_context(std::unique_ptr<mbedtls_ssl_context> context);

        /*!
         * Sets if the socket should verify the endpoints
         * certificates or not. Verification is enforced
         * by default, but disabling it could be useful
         * for testing.
         *
         * @param should_verify True if certificates should be verified, false otherwise
         */
        void verify_certificates(bool should_verify);

        /*!
         * Gets the underlying socket descriptor.
         *
         * @return The socket's descriptor. -1 indicates no connection.
         */
        inline int32_t get_socket_descriptor() const override
        {
            if(!ssl_socket_descriptor)
                return -1;
            return ssl_socket_descriptor->fd;
        }

        /*!
         * Sets if the socket should block or not.
         *
         * @note This must be set *WHILST* connected
         * @param should_block True to block, false otherwise.
         */
        inline void set_blocking(bool should_block) override
        {
            mbedtls_net_set_block(ssl_socket_descriptor.get());
        }

        /*!
         * Checks to see if we're connected to a socket or not
         *
         * @return True if it's connected. False otherwise.
         */
        inline bool connected() const final
        {
            return ssl_socket_descriptor && ssl_socket_descriptor->fd > -1;
        }


    private:

        /*!
         * Close the connection.
         */
        void close_socket() override;

        std::shared_ptr<SSLContext> ssl_context;
        std::unique_ptr<mbedtls_net_context> ssl_socket_descriptor;
        std::unique_ptr<mbedtls_ssl_context> ssl;
        mbedtls_ssl_config conf;
        uint32_t flags;
        bool should_verify;
    };
}

#endif //FRNETLIB_SSLSOCKET_H
