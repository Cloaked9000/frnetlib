//
// Created by fred on 12/12/16.
//

#include "TLSSocket.h"
#ifdef SSL_ENABLED

namespace fr
{
    TLSSocket::TLSSocket()
    {
        int error = 0;

        //Initialise mbedtls structures
        mbedtls_net_init(&ssl_socket_descriptor);
        mbedtls_ssl_init(&ssl);
        mbedtls_ssl_config_init(&conf);
        mbedtls_x509_crt_init(&cacert);
        mbedtls_ctr_drbg_init(&ctr_drbg);

        //Seed random number generator
        if((error = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, NULL, NULL)) != 0)
        {
            std::cout << "Failed to initialise random number generator. Returned error: " << error << std::endl;
            return;
        }

        //Load root CA certificate
        if((error = mbedtls_x509_crt_parse(&cacert, (const unsigned char *)mbedtls_test_cas_pem, mbedtls_test_cas_pem_len) < 0))
        {
            std::cout << "Failed to parse root CA certificate. Parse returned: " << error << std::endl;
            return;
        }
    }
    Socket::Status TLSSocket::send_raw(const char *data, size_t size)
    {
        return TcpSocket::send_raw(data, size);
    }

    Socket::Status TLSSocket::receive_raw(void *data, size_t data_size, size_t &received)
    {
        return TcpSocket::receive_raw(data, data_size, received);
    }

    void TLSSocket::set_descriptor(int descriptor)
    {
        TcpSocket::set_descriptor(descriptor);
    }

    void TLSSocket::close()
    {
        TcpSocket::close();
    }

    Socket::Status TLSSocket::connect(const std::string &address, const std::string &port)
    {
        return TcpSocket::connect(address, port);
    }
}

#endif