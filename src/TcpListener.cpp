//
// Created by fred on 06/12/16.
//

#include <frnetlib/TcpListener.h>

#include "frnetlib/TcpListener.h"

namespace fr
{

    const int yes = 1;
    const int no = 0;

    TcpListener::TcpListener()
    : socket_descriptor(-1)
    {

    }

    TcpListener::~TcpListener()
    {
        close_socket();
    }

    Socket::Status TcpListener::listen(const std::string &port)
    {
        addrinfo *info;
        addrinfo hints{};

        memset(&hints, 0, sizeof(addrinfo));

        hints.ai_family = ai_family;
        hints.ai_socktype = SOCK_STREAM; //TCP
        hints.ai_flags = AI_PASSIVE; //Have the IP filled in for us

        if(getaddrinfo(nullptr, port.c_str(), &hints, &info) != 0)
        {
            return Socket::Status::Unknown;
        }
        //Try each of the results until we listen successfully
        addrinfo *c = nullptr;
        for(c = info; c != nullptr; c = c->ai_next)
        {
            //Attempt to connect
            if((socket_descriptor = socket(c->ai_family, c->ai_socktype, c->ai_protocol)) == INVALID_SOCKET)
            {
                continue;
            }
            //Set address re-use option
            if(setsockopt(socket_descriptor, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(int)) == SOCKET_ERROR)
            {
                continue;
            }

            //If it's an IPv6 interface, attempt to allow IPv4 connections
            if(c->ai_family == AF_INET6)
            {
                setsockopt(socket_descriptor, IPPROTO_IPV6, IPV6_V6ONLY, (char*)&no, sizeof(no));
            }

            //Attempt to bind
            if(bind(socket_descriptor, c->ai_addr, c->ai_addrlen) == SOCKET_ERROR)
            {
                closesocket(socket_descriptor);
                continue;
            }
            break;
        }

        //Check that we've actually bound
        if(c == nullptr)
        {
            return Socket::Status::BindFailed;
        }

        //We're done with this now, cleanup
        freeaddrinfo(info);

        //Listen to socket
        if(::listen(socket_descriptor, LISTEN_QUEUE_SIZE) == SOCKET_ERROR)
        {
            return Socket::ListenFailed;
        }
        return Socket::Success;
    }

    Socket::Status TcpListener::accept(Socket &client_)
    {
        //Cast to TcpSocket. Will throw bad cast on failure.
        auto &client = dynamic_cast<TcpSocket&>(client_);

        //Prepare to wait for the client
        sockaddr_storage client_addr{};
        int32_t client_descriptor;
        char client_printable_addr[INET6_ADDRSTRLEN];

        //Accept one
        socklen_t client_addr_len = sizeof client_addr;
        client_descriptor = ::accept(socket_descriptor, (sockaddr*)&client_addr, &client_addr_len);
        if(client_descriptor == SOCKET_ERROR)
            return Socket::Unknown;

        //Get printable address. If we failed then set it as just 'unknown'
        int err = getnameinfo((sockaddr*)&client_addr, client_addr_len, client_printable_addr, sizeof(client_printable_addr), nullptr, 0, NI_NUMERICHOST);
        if(err != 0)
        {
            strcpy(client_printable_addr, "unknown");
        }

        //Set client data
        client.set_descriptor(&client_descriptor);
        client.set_remote_address(client_printable_addr);

        return Socket::Success;
    }

    void TcpListener::shutdown()
    {
        ::shutdown(socket_descriptor, 0);
    }

    int32_t TcpListener::get_socket_descriptor() const noexcept
    {
        return socket_descriptor;
    }

    void TcpListener::set_socket_descriptor(int32_t descriptor)
    {
        socket_descriptor = descriptor;
    }

    void TcpListener::close_socket()
    {
        if(socket_descriptor > -1)
        {
            closesocket(socket_descriptor);
            socket_descriptor = -1;
        }
    }

    bool TcpListener::connected() const noexcept
    {
        return socket_descriptor > -1;
    }
}