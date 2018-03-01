//
// Created by fred on 06/12/16.
//

#include <iostream>
#include <frnetlib/SocketSelector.h>
#include "frnetlib/TcpSocket.h"
#define DEFAULT_SOCKET_TIMEOUT 20

namespace fr
{

    TcpSocket::TcpSocket() noexcept
    : socket_descriptor(-1)
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
            int64_t status = ::send(socket_descriptor, data + sent, size - sent, 0);
            if(status > 0)
            {
                sent += status;
            }
            else if(errno != EWOULDBLOCK && errno != EAGAIN) //Don't exit if the socket just couldn't block
            {
                close_socket();
                return Socket::Status::Disconnected;
            }
        }
        return Socket::Status::Success;
    }

    void TcpSocket::close_socket()
    {
        if(socket_descriptor > -1)
        {
            ::closesocket(socket_descriptor);
            socket_descriptor = -1;
        }
    }

    Socket::Status TcpSocket::receive_raw(void *data, size_t buffer_size, size_t &received)
    {
        received = 0;

        //Read RECV_CHUNK_SIZE bytes into the recv buffer
		int64_t status = ::recv(socket_descriptor, (char*)data, buffer_size, 0);

        if(status > 0)
        {
            received += status;
        }
        else
        {
            if(errno == EWOULDBLOCK || errno == EAGAIN)
            {
                return Socket::Status::WouldBlock;
            }

            close_socket();
            return Socket::Status::Disconnected;
        }

        if(received > buffer_size)
            received = buffer_size;

        return Socket::Status::Success;
    }


    void TcpSocket::set_descriptor(void *descriptor)
    {
        if(!descriptor)
        {
            socket_descriptor = -1;
            return;
        }
        socket_descriptor = *static_cast<int32_t*>(descriptor);
        reconfigure_socket();
    }

    Socket::Status TcpSocket::connect(const std::string &address, const std::string &port, std::chrono::seconds timeout)
    {
        //Setup required structures
        int ret = 0;
        addrinfo *info;
        addrinfo hints{};

        memset(&hints, 0, sizeof(addrinfo));

        //Setup connection settings
        hints.ai_family = ai_family;
        hints.ai_socktype = SOCK_STREAM; //TCP
        hints.ai_flags = AI_PASSIVE; //Have the IP filled in for us

        //Query remote address information
        if(getaddrinfo(address.c_str(), port.c_str(), &hints, &info) != 0)
        {
            return Socket::Status::Error;
        }

        //Try to connect to results returned by getaddrinfo until we succeed/run out of things
        addrinfo *c;
        for(c = info; c != nullptr; c = c->ai_next)
        {
            //Get the socket for this entry
            socket_descriptor = ::socket(c->ai_family, c->ai_socktype, c->ai_protocol);
            if(socket_descriptor == INVALID_SOCKET)
                continue;


            //Put it into non-blocking mode, to allow for a custom connect timeout
            if(!set_unix_socket_blocking(socket_descriptor, true, false))
                continue;

            //Try and connect
            ret = ::connect(socket_descriptor, c->ai_addr, c->ai_addrlen);
            if(ret < 0 && errno != EINPROGRESS)
                continue;
            else if(ret == 0) //If it connected immediately then break out of the connect loop
                break;

            //Wait for the socket to do something/expire
            timeval tv = {};
            tv.tv_sec = timeout.count() == 0 ? DEFAULT_SOCKET_TIMEOUT : timeout.count();
            tv.tv_usec = 0;
            fd_set set = {};
            FD_ZERO(&set);
            FD_SET(socket_descriptor, &set);
            ret = select(socket_descriptor + 1, nullptr, &set, nullptr, &tv);
            if(ret <= 0)
                continue;

            //Verify that we're connected
            socklen_t len = sizeof(ret);
            if(getsockopt(socket_descriptor, SOL_SOCKET, SO_ERROR, (char*)&ret, &len) == -1)
                continue;
            if(ret != 0)
                continue;

            break;
        }

        //We're done with this now, cleanup
        freeaddrinfo(info);
        if(c == nullptr)
            return Socket::Status::Error;

        //Turn back to blocking mode
        if(!set_unix_socket_blocking(socket_descriptor, false, true))
            return Socket::Status::Error;

        //Update state now we've got a valid socket descriptor
        remote_address = address + ":" + port;
        reconfigure_socket();

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