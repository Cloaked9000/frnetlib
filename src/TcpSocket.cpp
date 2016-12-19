//
// Created by fred on 06/12/16.
//

#include <iostream>
#include "frnetlib/TcpSocket.h"

namespace fr
{

    TcpSocket::TcpSocket() noexcept
    : recv_buffer(new char[RECV_CHUNK_SIZE])
    {

    }

    TcpSocket::~TcpSocket() noexcept
    {
        close_socket();
    }

    Socket::Status TcpSocket::send_raw(const char *data, size_t size)
    {
        size_t sent = 0;
        while(sent < size)
        {
            ssize_t status = ::send(socket_descriptor, data + sent, size - sent, 0);
            if(status > 0)
            {
                sent += status;
            }
            else if(errno != EWOULDBLOCK && errno != EAGAIN) //Don't exit if the socket just couldn't block
            {
                if(status == -1)
                {
                    return Socket::Status::Error;
                }

                is_connected = false;
                return Socket::Status::Disconnected;
            }
        }
        return Socket::Status::Success;
    }

    void TcpSocket::close_socket()
    {
        if(is_connected)
        {
            ::closesocket(socket_descriptor);
            is_connected = false;
        }
    }

    Socket::Status TcpSocket::receive_raw(void *data, size_t data_size, size_t &received)
    {
        received = 0;
        if(unprocessed_buffer.size() < data_size)
        {
            //Read RECV_CHUNK_SIZE bytes into the recv buffer
            ssize_t status = ::recv(socket_descriptor, recv_buffer.get(), RECV_CHUNK_SIZE, 0);

            if(status > 0)
            {
                unprocessed_buffer += {recv_buffer.get(), (size_t)status};
                received += status;
            }
            else
            {
                if(errno == EWOULDBLOCK || errno == EAGAIN)
                {
                    return Socket::Status::WouldBlock;
                }
                else if(status == -1)
                {
                    return Socket::Status::Error;
                }

                is_connected = false;
                return Socket::Status::Disconnected;
            }

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


    void TcpSocket::set_descriptor(int descriptor)
    {
        socket_descriptor = descriptor;
        is_connected = true;
    }

    Socket::Status TcpSocket::connect(const std::string &address, const std::string &port)
    {
        remote_address = address + ":" + port;

        addrinfo *info;
        addrinfo hints;

        memset(&hints, 0, sizeof(addrinfo));

        hints.ai_family = AF_UNSPEC; //IPv6 or IPv4
        hints.ai_socktype = SOCK_STREAM; //TCP
        hints.ai_flags = AI_PASSIVE; //Have the IP filled in for us

        if(getaddrinfo(address.c_str(), port.c_str(), &hints, &info) != 0)
        {
            return Socket::Status::Error;
        }

        addrinfo *c;
        for(c = info; c != nullptr; c = c->ai_next)
        {
            socket_descriptor = ::socket(c->ai_family, c->ai_socktype, c->ai_protocol);
            if(socket_descriptor == INVALID_SOCKET)
            {
                continue;
            }

            if(::connect(socket_descriptor, c->ai_addr, c->ai_addrlen) == SOCKET_ERROR)
            {
                continue;
            }

            break;
        }

        if(c == nullptr)
            return Socket::Status::Error;

        //We're done with this now, cleanup
        freeaddrinfo(info);

        is_connected = true;
        return Socket::Status::Success;
    }

    void TcpSocket::set_blocking(bool should_block)
    {
        set_unix_socket_blocking(socket_descriptor, is_blocking, should_block);
        is_blocking = should_block;
    }

    int32_t TcpSocket::get_socket_descriptor() const
    {
        return socket_descriptor;
    }

}