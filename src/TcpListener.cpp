//
// Created by fred on 06/12/16.
//

#include "frnetlib/TcpListener.h"

namespace fr
{

    const int yes = 1;
    const int no = 0;

    Socket::Status TcpListener::listen(const std::string &port)
    {
        addrinfo *info;
        addrinfo hints;

        memset(&hints, 0, sizeof(addrinfo));

        hints.ai_family = AF_UNSPEC; //IPv6 or IPv4. NOTE: Might want to make configurable.
        hints.ai_socktype = SOCK_STREAM; //TCP
        hints.ai_flags = AI_PASSIVE; //Have the IP filled in for us

        if(getaddrinfo(NULL, port.c_str(), &hints, &info) != 0)
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
        if(::listen(socket_descriptor, 10) == SOCKET_ERROR)
        {
            return Socket::ListenFailed;
        }
        return Socket::Success;
    }

    Socket::Status TcpListener::accept(Socket &client_)
    {
        //Cast to TcpSocket. Will throw bad cast on failure.
        TcpSocket &client = dynamic_cast<TcpSocket&>(client_);

        //Prepare to wait for the client
        sockaddr_storage client_addr;
        int client_descriptor;
        char client_printable_addr[INET6_ADDRSTRLEN];

        //Accept one
        socklen_t client_addr_len = sizeof client_addr;
        client_descriptor = ::accept(socket_descriptor, (sockaddr*)&client_addr, &client_addr_len);
        if(client_descriptor == SOCKET_ERROR)
            return Socket::Unknown;

        //Get printable address. If we failed then set it as just 'unknown'
        int err = getnameinfo((sockaddr*)&client_addr, client_addr_len, client_printable_addr, sizeof(client_printable_addr), 0,0,NI_NUMERICHOST);
        if(err != 0)
            strcpy(client_printable_addr, "unknown");

        //Set client data
        client.set_descriptor(client_descriptor);
        client.set_remote_address(client_printable_addr);

        return Socket::Success;
    }

    void TcpListener::shutdown()
    {
        ::shutdown(socket_descriptor, 0);
    }
}