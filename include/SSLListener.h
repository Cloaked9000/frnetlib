//
// Created by fred on 13/12/16.
//

#ifndef FRNETLIB_SSLLISTENER_H
#define FRNETLIB_SSLLISTENER_H

#define SSL_ENABLED

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
        SSLListener() noexcept;
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

    private:
        mbedtls_net_context listen_fd;
        mbedtls_entropy_context entropy;
        mbedtls_ctr_drbg_context ctr_drbg;
        mbedtls_ssl_config conf;
        mbedtls_x509_crt srvcert;
        mbedtls_pk_context pkey;

        //Stubs
        virtual Status send(const Packet &packet){return Socket::Error;}
        virtual Status receive(Packet &packet){return Socket::Error;}
        virtual void close(){}
        virtual Socket::Status connect(const std::string &address, const std::string &port){return Socket::Error;}
    };

}

#endif //SLL_ENABLED
#endif //FRNETLIB_SSLLISTENER_H
