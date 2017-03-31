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

#include "SSLSocket.h"
#include "Listener.h"


namespace fr
{
    class SSLListener : public Listener
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
        virtual Socket::Status listen(const std::string &port) override;

        /*!
         * Accepts a new connection.
         *
         * @param client Where to store the connection information
         * @return True on success. False on failure.
         */
        virtual Socket::Status accept(Socket &client) override;

    private:
        mbedtls_net_context listen_fd;
        mbedtls_ssl_config conf;
        mbedtls_x509_crt srvcert;
        mbedtls_pk_context pkey;

        std::shared_ptr<SSLContext> ssl_context;
    };

}

#endif //SLL_ENABLED
#endif //FRNETLIB_SSLLISTENER_H
