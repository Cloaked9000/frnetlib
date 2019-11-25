//
// Created by fred on 06/12/16.
//

#include <iostream>
#include <frnetlib/SocketSelector.h>
#include <frnetlib/TcpSocket.h>
#define DEFAULT_SOCKET_TIMEOUT 20

namespace fr
{

    TcpSocket::TcpSocket() noexcept
    : socket_descriptor(-1),
      is_blocking(true)
    {

    }

    TcpSocket::~TcpSocket()
    {
        TcpSocket::close_socket();
    }

    Socket::Status TcpSocket::send_raw(const char *data, size_t size, size_t &sent)
    {
        while(sent < size)
        {
            int64_t status = ::send(socket_descriptor, data + sent, size - sent, 0);
            if(status >= 0)
            {
                sent += status;
                continue;
            }

            if(errno == EWOULDBLOCK)
            {
                if(is_blocking)
                {
                    return Socket::Status::Timeout;
                }
                return Socket::Status::WouldBlock;
            }
            else if(errno == EINTR)
            {
                continue; //try again, interrupted before anything could be received
            }

            return Socket::Status::SendError;
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
        ssize_t status = 0;
        do
        {
            status = ::recv(socket_descriptor, (char*)data, buffer_size, 0);
            if(status == 0)
            {
                return Socket::Status::Disconnected;
            }

            if(status < 0)
            {
                if(errno == EWOULDBLOCK)
                {
                    if(is_blocking)
                    {
                        return Socket::Status::Timeout;
                    }
                    return Socket::Status::WouldBlock;
                }
                else if(errno == EINTR)
                {
                    continue; //try again, interrupted before anything could be received
                }

                return Socket::Status::ReceiveError;
            }
            break;
        } while(true);


        received = static_cast<size_t>(status);
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
        if((ret = getaddrinfo(address.c_str(), port.c_str(), &hints, &info)) != 0)
        {
            errno = ret;
            return Socket::Status::AddressLookupFailure;
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
#ifdef _WIN32
            if(ret < 0 && WSAGetLastError() != WSAEWOULDBLOCK)
#else
            if(ret < 0 && errno != EINPROGRESS)
#endif
            {
                continue;
            }
            else if(ret == 0) //If it connected immediately then break out of the connect loop
            {
                break;
            }

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
            return Socket::Status::NoRouteToHost;

        //Turn back to blocking mode
        if(!set_unix_socket_blocking(socket_descriptor, false, true))
            return Socket::Status::Error;

        //Update state now we've got a valid socket descriptor
        set_remote_address(address + ":" + port);
        reconfigure_socket();

        return Socket::Status::Success;
    }

    Socket::Status TcpSocket::set_blocking(bool should_block)
    {
        if(!set_unix_socket_blocking(socket_descriptor, is_blocking, should_block))
            return Status::Error;
        is_blocking = should_block;
        return fr::Socket::Status::Success;
    }

    int32_t TcpSocket::get_socket_descriptor() const noexcept
    {
        return socket_descriptor;
    }

    void TcpSocket::reconfigure_socket()
    {
        if(!connected())
        {
            return;
        }

        int one = 1;
#ifndef _WIN32
        //Disable Nagle's algorithm
        setsockopt(get_socket_descriptor(), SOL_TCP, TCP_NODELAY, (char*)&one, sizeof(one));

        //Apply receive timeout
        struct timeval tv = {};
        tv.tv_sec = get_receive_timeout() / 1000;
        tv.tv_usec = (get_receive_timeout() % 1000) * 1000;
        setsockopt(socket_descriptor, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

        //Apply send timeout
        tv.tv_sec = get_send_timeout() / 1000;
        tv.tv_usec = (get_send_timeout() % 1000) * 1000;
        setsockopt(socket_descriptor, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof(tv));
#else
        //Disable Nagle's algorithm
        setsockopt(get_socket_descriptor(), IPPROTO_TCP, TCP_NODELAY, (char*)&one, sizeof(one));
        setsockopt(get_socket_descriptor(), SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char*)&one, sizeof(one));

        //Apply receive timeout
        DWORD timeout_dword = static_cast<DWORD>(get_receive_timeout());
        setsockopt(socket_descriptor, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout_dword, sizeof timeout_dword);

        //Apply send timeout
        timeout_dword = static_cast<DWORD>(get_send_timeout());
        setsockopt(socket_descriptor, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout_dword, sizeof timeout_dword);
#endif
    }

    bool TcpSocket::connected() const
    {
        return socket_descriptor > -1;
    }

    bool TcpSocket::get_blocking() const
    {
        return is_blocking;
    }


}