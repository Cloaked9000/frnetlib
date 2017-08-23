//
// Created by fred on 06/12/16.
//

#include <mutex>
#include <csignal>
#include <iostream>
#include "frnetlib/NetworkEncoding.h"
#include "frnetlib/Socket.h"
#include "frnetlib/Sendable.h"

namespace fr
{
    Socket::Socket() noexcept
    : is_blocking(true),
      ai_family(AF_UNSPEC),
      max_receive_size(0)
    {
        init_wsa();
    }

    Socket::Status Socket::send(Sendable &obj)
    {
        if(!connected())
            return Socket::Disconnected;

        return obj.send(this);
    }

    Socket::Status Socket::send(Sendable &&obj)
    {
        if(!connected())
            return Socket::Disconnected;

        return obj.send(this);
    }

    Socket::Status Socket::receive(Sendable &obj)
    {
        if(!connected())
            return Socket::Disconnected;

        std::lock_guard<std::mutex> guard(inbound_mutex);
        return obj.receive(this);
    }

    Socket::Status Socket::receive_all(void *dest, size_t buffer_size)
    {
        if(!connected())
            return Socket::Disconnected;

        auto bytes_remaining = (int32_t) buffer_size;
        size_t bytes_read = 0;
        while(bytes_remaining > 0)
        {
            size_t received = 0;
            auto *arr = (char*)dest;
            Status status = receive_raw(&arr[bytes_read], (size_t)bytes_remaining, received);
            if(status != fr::Socket::Success)
                return status;
            bytes_remaining -= received;
            bytes_read += received;
        }

        return Socket::Status::Success;
    }

    void Socket::shutdown()
    {
        ::shutdown(get_socket_descriptor(), 0);
    }

    void Socket::reconfigure_socket()
    {
        //todo: Perhaps allow for these settings to be modified
        int one = 1;
        setsockopt(get_socket_descriptor(), SOL_TCP, TCP_NODELAY, (char*)&one, sizeof(one));
#ifdef _WIN32
        setsockopt(get_socket_descriptor(), SOL_SOCKET, SO_EXCLUSIVEADDRUSE, (char*)&one, sizeof(one));
#endif
    }

    void Socket::set_inet_version(Socket::IP version)
    {
        switch(version)
        {
            case Socket::IP::v4:
                ai_family = AF_INET;
                break;
            case Socket::IP::v6:
                ai_family = AF_INET6;
                break;
            case Socket::IP::any:
                ai_family = AF_UNSPEC;
                break;
            default:
                throw std::logic_error("Unknown Socket::IP value passed to set_inet_version()");
        }
    }

    void Socket::set_max_receive_size(uint32_t sz)
    {
        max_receive_size = sz;
    }
}