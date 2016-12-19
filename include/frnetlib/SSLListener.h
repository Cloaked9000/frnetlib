//
// Created by fred on 13/12/16.
//

#ifndef FRNETLIB_SSLLISTENER_H
#define FRNETLIB_SSLLISTENER_H

#ifdef SSL_ENABLED

#include <mbedtls/net_sockets.h>
#include <mbedtls/debug.h>
#include <mbedtls/ssl.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/error.h>
#include <mbedtls/certs.h>

#include "TcpListener.h"
#include "SSLSocket.h"


namespace fr
{
    class SSLListener : public Socket
    {
    public:
        SSLListener(std::shared_ptr<SSLContext> ssl_context, const std::string &crt_path, const std::string &pem_path, const std::string &private_key_path) noexcept;
        virtual ~SSLListener() noexcept;
        SSLListener(SSLListener &&o) noexcept = default;

        /*!
         * Listens to the given port for connections
         *
         * @param port The port to bind to
         * @return If the operation was successful
         */
        virtual Socket::Status listen(const std::string &port);

        /*!
         * Accepts a new connection.
         *
         * @param client Where to store the connection information
         * @return True on success. False on failure.
         */
        virtual Socket::Status accept(SSLSocket &client);

        /*!
         * Enables or disables blocking on the socket.
         *
         * @param should_block True to block, false otherwise.
         */
        virtual void set_blocking(bool should_block) override {abort();}; //Not implemented

        virtual int32_t get_socket_descriptor() const override
        {
            return listen_fd.fd;
        }

    private:
        mbedtls_net_context listen_fd;
        mbedtls_ssl_config conf;
        mbedtls_x509_crt srvcert;
        mbedtls_pk_context pkey;

        std::shared_ptr<SSLContext> ssl_context;

        //Stubs
        virtual void close_socket(){}
        virtual Socket::Status connect(const std::string &address, const std::string &port){return Socket::Error;}
        virtual Status send_raw(const char *data, size_t size) {return Socket::Error;}
        virtual Status receive_raw(void *data, size_t data_size, size_t &received) {return Socket::Error;}
    };

}

#endif //SLL_ENABLED
#endif //FRNETLIB_SSLLISTENER_H
