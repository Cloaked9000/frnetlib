//
// Created by fred on 12/12/16.
//

#ifndef FRNETLIB_TLSSOCKET_H
#define FRNETLIB_TLSSOCKET_H



#ifdef SSL_ENABLED

#include "TcpSocket.h"
#include <mbedtls/net_sockets.h>
#include <mbedtls/debug.h>
#include <mbedtls/ssl.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/error.h>
#include <mbedtls/certs.h>

namespace fr
{
    class TLSSocket : public TcpSocket
    {
    public:
        TLSSocket();

        /*!
         * Effectively just fr::TcpSocket::send_raw() with encryption
         * added in.
         *
         * @param data The data to send.
         * @param size The number of bytes, from data to send. Be careful not to overflow.
         * @return The status of the operation.
         */
        Status send_raw(const char *data, size_t size) override;


        /*!
         * Effectively just fr::TcpSocket::receive_raw() with encryption
         * added in.
         *
         * @param data Where to store the received data.
         * @param data_size The number of bytes to try and receive. Be sure that it's not larger than data.
         * @param received Will be filled with the number of bytes actually received, might be less than you requested.
         * @return The status of the operation, if the socket has disconnected etc.
         */
        Status receive_raw(void *data, size_t data_size, size_t &received) override;

        /*!
         * Sets the socket file descriptor.
         *
         * @param descriptor The socket descriptor.
         */
        void set_descriptor(int descriptor) override;

        /*!
         * Close the connection.
         */
        void close() override;

        /*!
         * Connects the socket to an address.
         *
         * @param address The address of the socket to connect to
         * @param port The port of the socket to connect to
         * @return A Socket::Status indicating the status of the operation.
         */
        Socket::Status connect(const std::string &address, const std::string &port) override;

    private:
        mbedtls_net_context ssl_socket_descriptor;
        mbedtls_entropy_context entropy;
        mbedtls_ctr_drbg_context ctr_drbg;
        mbedtls_ssl_context ssl;
        mbedtls_ssl_config conf;
        mbedtls_x509_crt cacert;
    };
}

#endif

#endif //FRNETLIB_TLSSOCKET_H
