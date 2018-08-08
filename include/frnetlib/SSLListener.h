//
// Created by fred on 13/12/16.
//

#ifndef FRNETLIB_SSLLISTENER_H
#define FRNETLIB_SSLLISTENER_H

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
        explicit SSLListener(std::shared_ptr<SSLContext> ssl_context, const std::string &pem_path, const std::string &private_key_path);
        virtual ~SSLListener() noexcept;
        SSLListener(SSLListener &&) = delete;
        SSLListener(SSLListener &o) = delete;
        void operator=(const SSLListener &) = delete;
        void operator=(SSLListener &&) = delete;

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

        /*!
         * Closes the socket
         */
        void close_socket() override;

        /*!
         * Calls the shutdown syscall on the socket.
         * So you can receive data but not send.
         *
         * This can be called on a blocking socket to force
         * it to immediately return (you might want to do this if
         * you're exiting and need the blocking socket to return).
         */
        void shutdown() override;

        /*!
         * Gets the socket descriptor.
         *
         * @return The listen socket descriptor
         */
        int32_t get_socket_descriptor() const override;

        /*!
         * Sets the socket descriptor.
         *
         * @param descriptor The listen descriptor to use
         */
        void set_socket_descriptor(int32_t descriptor) override;

    private:
        mbedtls_net_context listen_fd;
        mbedtls_ssl_config conf;
        mbedtls_x509_crt srvcert;
        mbedtls_pk_context pkey;

        std::shared_ptr<SSLContext> ssl_context;
    };

}
#endif //FRNETLIB_SSLLISTENER_H
