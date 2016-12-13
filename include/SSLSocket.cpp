//
// Created by fred on 12/12/16.
//

#include "SSLSocket.h"
#ifdef SSL_ENABLED

namespace fr
{
    SSLSocket::SSLSocket()
    {
        int error = 0;
        const char *pers = "ssl_client1";

        //Initialise mbedtls structures
        mbedtls_net_init(&ssl_socket_descriptor);
        mbedtls_ssl_init(&ssl);
        mbedtls_ssl_config_init(&conf);
        mbedtls_x509_crt_init(&cacert);
        mbedtls_ctr_drbg_init(&ctr_drbg);

        //Seed random number generator
        mbedtls_entropy_init(&entropy);
        if((error = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char *)pers, strlen(pers))) != 0)
        {
            std::cout << "Failed to initialise random number generator. Returned error: " << error << std::endl;
            return;
        }

        //Load root CA certificate
        if((error = mbedtls_x509_crt_parse(&cacert, (const unsigned char *)certs.c_str(), certs.size() + 1) < 0))
        {
            std::cout << "Failed to parse root CA certificate. Parse returned: " << error << std::endl;
            return;
        }
    }

    SSLSocket::~SSLSocket()
    {
        //Close connection if active
        close();

        //Cleanup mbedsql stuff
        mbedtls_net_free(&ssl_socket_descriptor);
        mbedtls_x509_crt_free(&cacert);
        mbedtls_ssl_free(&ssl);
        mbedtls_ssl_config_free(&conf);
        mbedtls_ctr_drbg_free(&ctr_drbg);
        mbedtls_entropy_free(&entropy);
    }

    void SSLSocket::close()
    {
        if(is_connected)
        {
            mbedtls_ssl_close_notify(&ssl);
            is_connected = false;
        }
    }

    Socket::Status SSLSocket::send_raw(const char *data, size_t size)
    {
        int error = 0;
        while((error = mbedtls_ssl_write(&ssl, (const unsigned char *)data, size)) <= 0)
        {
            if(error != MBEDTLS_ERR_SSL_WANT_READ && error != MBEDTLS_ERR_SSL_WANT_WRITE)
            {
                return Socket::Status::Error;
            }
        }

        return Socket::Status::Success;
    }

    Socket::Status SSLSocket::receive_raw(void *data, size_t data_size, size_t &received)
    {
        int read = 0;
        received = 0;
        if(unprocessed_buffer.size() < data_size)
        {
            read = mbedtls_ssl_read(&ssl, (unsigned char *)recv_buffer.get(), RECV_CHUNK_SIZE);

            if(read == MBEDTLS_ERR_SSL_WANT_READ || read == MBEDTLS_ERR_SSL_WANT_WRITE)
            {
                received = 0;
                return Socket::Status::Success;
            }
            else if(read == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY)
            {
                std::cout << "disconnected" << std::endl;
                return Socket::Status::Disconnected;
            }
            else if(read <= 0)
            {
                std::cout << "read <= 0" << std::endl;
                return Socket::Status::Error;
            }

            received += read;
            unprocessed_buffer += {recv_buffer.get(), (size_t)read};

            if(received > data_size)
                received = data_size;
        }
        else
        {
            received = data_size;
        }

        //Copy data to where it needs to go
        memcpy(data, &unprocessed_buffer[0], received);
        unprocessed_buffer.erase(0, received);
        return Socket::Status::Success;

    }

    void SSLSocket::set_descriptor(int descriptor)
    {
        is_connected = true;
        socket_descriptor = descriptor;
        ssl_socket_descriptor.fd = descriptor;
    }

    Socket::Status SSLSocket::connect(const std::string &address, const std::string &port)
    {
        //Initialise the connection using mbedtls
        int error = 0;
        if((error = mbedtls_net_connect(&ssl_socket_descriptor, address.c_str(), port.c_str(), MBEDTLS_NET_PROTO_TCP)) != 0)
        {
            return Socket::Status::ConnectionFailed;
        }

        //Initialise SSL data structures
        if((error = mbedtls_ssl_config_defaults(&conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT)) != 0)
        {
            return Socket::Status::Error;
        }

        mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_REQUIRED);
        mbedtls_ssl_conf_ca_chain(&conf, &cacert, NULL);
        mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);

        if((error = mbedtls_ssl_setup(&ssl, &conf)) != 0)
        {
            return Socket::Status::Error;
        }

        if((error = mbedtls_ssl_set_hostname(&ssl, address.c_str())) != 0)
        {
            return Socket::Status::Error;
        }

        mbedtls_ssl_set_bio(&ssl, &ssl_socket_descriptor, mbedtls_net_send, mbedtls_net_recv, NULL);

        //Do SSL handshake
        while((error = mbedtls_ssl_handshake(&ssl)) != 0)
        {
            if(error != MBEDTLS_ERR_SSL_WANT_READ && error != MBEDTLS_ERR_SSL_WANT_WRITE)
            {
                std::cout << "Failed to connect to server. Handshake returned: " << error << std::endl;
                return Socket::Status::HandshakeFailed;
            }
        }

        //Verify server certificate
        if((flags = mbedtls_ssl_get_verify_result(&ssl)) != 0)
        {
            char verify_buffer[512];
            mbedtls_x509_crt_verify_info( verify_buffer, sizeof( verify_buffer ), "  ! ", flags );

            std::cout << "Failed to connect to server. Server certificate validation failed: " << verify_buffer << std::endl;
            return Socket::Status::VerificationFailed;
        }

        //Update members
        is_connected = true;
        remote_address = address + ":" + port;
        return Socket::Status::Success;
    }
}

#endif