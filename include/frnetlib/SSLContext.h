//
// Created by fred on 16/12/16.
//

#ifndef FRNETLIB_SSLCONTEXT_H
#define FRNETLIB_SSLCONTEXT_H

#include <mbedtls/x509_crt.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <cstring>
#include <iostream>

namespace fr
{
    class SSLContext
    {
    public:
        SSLContext(SSLContext&)=delete;
        SSLContext(SSLContext&&)=delete;
        void operator=(SSLContext&&)=delete;
        void operator=(const SSLContext&)=delete;

        /*!
         * Initialises a new SSL context for use with SSL instances.
         * Will throw a std::runtime_error on failure.
         */
        SSLContext()
        {
            int error = 0;

            //Initialise mbed_tls structures
            mbedtls_x509_crt_init(&cacert);
            mbedtls_ctr_drbg_init(&ctr_drbg);

            //Seed random number generator
            mbedtls_entropy_init(&entropy);
            if((error = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, nullptr, 0)) != 0)
            {
                throw std::runtime_error("Failed to initialise random number generator. Returned error: " + std::to_string(error));
            }
        }

        ~SSLContext()
        {
            mbedtls_ctr_drbg_free(&ctr_drbg);
            mbedtls_entropy_free(&entropy);
            mbedtls_x509_crt_free(&cacert);
        }

        /*!
         * Parses a list of x509 crt certificates from a location in memory. May be called
         * multiple times to add more certificates to the chain.
         *
         * @param ca_certs The certificates to parse
         * @return True on success, false on failure.
         */
        bool load_ca_certs_from_memory(const std::string &ca_certs)
        {
            return mbedtls_x509_crt_parse(&cacert, (const unsigned char *)ca_certs.c_str(), ca_certs.size()) == 0;
        }

        /*!
         * Parses a list of x509 crt certificates from a location on disk. May be called
         * multiple times to add more certificates to the chain.
         *
         * @param ca_certs_filepath The certificates to parse
         * @return True on success, false on failure
         */
        bool load_ca_certs_from_file(const std::string &ca_certs_filepath)
        {
            return mbedtls_x509_crt_parse_file(&cacert, ca_certs_filepath.c_str()) == 0;
        }

        mbedtls_entropy_context entropy;
        mbedtls_ctr_drbg_context ctr_drbg;
        mbedtls_x509_crt cacert;

    };
    
    std::shared_ptr<SSLContext> GetTheContext();
}
#endif //FRNETLIB_SSLCONTEXT_H
