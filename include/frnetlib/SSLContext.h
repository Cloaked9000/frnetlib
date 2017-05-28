//
// Created by fred on 16/12/16.
//

#ifndef FRNETLIB_SSLCONTEXT_H
#define FRNETLIB_SSLCONTEXT_H

#ifdef SSL_ENABLED

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
         * Parses a list of x509 crt certificates from a location in memory
         *
         * @param ca_certs The certificates to parse
         * @return True on success, false on failure.
         */
        bool load_ca_certs_from_memory(const std::string &ca_certs)
        {
			std::cerr << "Note: load_ca_certs_from_memory() seems to be broken. Please use load_ca_certs_from_file() until this is resolved." << std::endl;
            int error = mbedtls_x509_crt_parse(&cacert, (const unsigned char *)ca_certs.c_str(), ca_certs.size());
            if(error < 0)
            {
                std::cout << "Failed to parse root CA certificates. Parse returned: " << error << std::endl;
                return false;
            }
            return true;
        }

        /*!
         * Parses a list of x509 crt certificates from a location on disk
         *
         * @param ca_certs_filepath The certificates to parse
         * @return True on success, false on failure
         */
        bool load_ca_certs_from_file(const std::string &ca_certs_filepath)
        {
            int error = mbedtls_x509_crt_parse_file(&cacert, ca_certs_filepath.c_str());
            if(error < 0)
            {
                std::cout << "Failed to parse root CA certificates. Parse returned: " << error << std::endl;
                return false;
            }
            return true;
        }

        mbedtls_entropy_context entropy;
        mbedtls_ctr_drbg_context ctr_drbg;
        mbedtls_x509_crt cacert;

    };
}
#endif // SSL_ENABLED


#endif //FRNETLIB_SSLCONTEXT_H
