//
// Created by fred on 16/12/16.
//

#ifndef FRNETLIB_SSLCONTEXT_H
#define FRNETLIB_SSLCONTEXT_H

#define USE_SSL
#ifdef USE_SSL

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
        SSLContext(const std::string &ca_certs_path)
        {
            int error = 0;

            //Initialise mbed_tls structures
            mbedtls_x509_crt_init(&cacert);
            mbedtls_ctr_drbg_init(&ctr_drbg);

            //Seed random number generator
            mbedtls_entropy_init(&entropy);
            if((error = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, nullptr, 0)) != 0)
            {
                std::cout << "Failed to initialise random number generator. Returned error: " << error << std::endl;
                return;
            }

            //Load root CA certificate
            if((error = mbedtls_x509_crt_parse_file(&cacert, ca_certs_path.c_str()) < 0))
            {
                std::cout << "Failed to parse root CA certificates. Parse returned: " << error << std::endl;
                return;
            }
        }

        ~SSLContext()
        {
            mbedtls_ctr_drbg_free(&ctr_drbg);
            mbedtls_entropy_free(&entropy);
            mbedtls_x509_crt_free(&cacert);
        }

        mbedtls_entropy_context entropy;
        mbedtls_ctr_drbg_context ctr_drbg;
        mbedtls_x509_crt cacert;

    };
}
#endif // USE_SSSL


#endif //FRNETLIB_SSLCONTEXT_H
