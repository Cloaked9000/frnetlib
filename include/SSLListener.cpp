//
// Created by fred on 13/12/16.
//

#include "SSLListener.h"

namespace fr
{
    SSLListener::SSLListener() noexcept
    {
        //Initialise SSL objects required
        mbedtls_net_init(&listen_fd);
        mbedtls_ssl_init(&ssl);
        mbedtls_ssl_config_init(&conf);
        mbedtls_x509_crt_init(&srvcert);
        mbedtls_pk_init(&pkey);
        mbedtls_entropy_init(&entropy);
        mbedtls_ctr_drbg_init(&ctr_drbg);

        int error = 0;

        //Load certificates and private key todo: Switch from inbuilt test certificates
        error = mbedtls_x509_crt_parse(&srvcert, (const unsigned char *)mbedtls_test_srv_crt, mbedtls_test_srv_crt_len);
        if(error != 0)
        {
            std::cout << "Failed to initialise SSL listener. CRT Parse returned: " << error << std::endl;
            return;
        }

        error = mbedtls_x509_crt_parse(&srvcert, (const unsigned char *) mbedtls_test_cas_pem, mbedtls_test_cas_pem_len);
        if(error != 0)
        {
            std::cout << "Failed to initialise SSL listener. PEM Parse returned: " << error << std::endl;
            return;
        }

        error =  mbedtls_pk_parse_key(&pkey, (const unsigned char *) mbedtls_test_srv_key, mbedtls_test_srv_key_len, NULL, 0);
        if(error != 0)
        {
            std::cout << "Failed to initialise SSL listener. Private Key Parse returned: " << error << std::endl;
            return;
        }

        //Seed random number generator
        if((error = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, NULL, 0)) != 0)
        {
            std::cout << "Failed to initialise SSL listener. Failed to seed random number generator: " << error << std::endl;
            return;
        }

        //Setup data structures
        if((error = mbedtls_ssl_config_defaults(&conf, MBEDTLS_SSL_IS_SERVER, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
        {
            std::cout << "Failed to configure SSL presets: " << error << std::endl;
            return;
        }

        //Apply them
        mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);
        mbedtls_ssl_conf_ca_chain(&conf, srvcert.next, NULL);

        if((error = mbedtls_ssl_conf_own_cert(&conf, &srvcert, &pkey)) != 0)
        {
            std::cout << "Failed to set certificate: " << error << std::endl;
            return;
        }

        if((error = mbedtls_ssl_setup( &ssl, &conf ) ) != 0)
        {
            std::cout << "Failed to apply SSL setings: " << error << std::endl;
            return;
        }

    }

    SSLListener::~SSLListener()
    {
        mbedtls_net_free(&listen_fd);
        mbedtls_x509_crt_free(&srvcert);
        mbedtls_pk_free(&pkey);
        mbedtls_ssl_free(&ssl);
        mbedtls_ssl_config_free(&conf);
        mbedtls_ctr_drbg_free(&ctr_drbg);
        mbedtls_entropy_free( &entropy);
    }

    Socket::Status fr::SSLListener::listen(const std::string &port)
    {
        //Bind to port
        if(mbedtls_net_bind( &listen_fd, NULL, port.c_str(), MBEDTLS_NET_PROTO_TCP) != 0)
        {
            return Socket::BindFailed;
        }
        return Socket::Success;
    }

    Socket::Status SSLListener::accept(SSLSocket &client)
    {
        int error = 0;

        //Accept a connection
        mbedtls_net_context client_fd;
        mbedtls_net_init(&client_fd);

        if((error = mbedtls_net_accept(&listen_fd, &client_fd, NULL, 0, NULL)) != 0)
        {
            std::cout << "Accept error: " << error << std::endl;
            return Socket::Error;
        }

        mbedtls_ssl_set_bio( &ssl, &client_fd, mbedtls_net_send, mbedtls_net_recv, NULL);

        //SSL Handshake
        while((error = mbedtls_ssl_handshake(&ssl)) != 0)
        {
            if(error != MBEDTLS_ERR_SSL_WANT_READ && error != MBEDTLS_ERR_SSL_WANT_WRITE)
            {
                std::cout << "Handshake failed: " << error << std::end
                return Socket::Status::HandshakeFailed;
            }
        }

        //Set socket details
        client.set_descriptor(client_fd.fd);
        return Socket::Success;
    }

}