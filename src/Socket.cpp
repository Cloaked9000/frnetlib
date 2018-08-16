//
// Created by fred on 06/12/16.
//

#include <mutex>
#include <csignal>
#include <iostream>
#include <vector>
#include "frnetlib/NetworkEncoding.h"
#include "frnetlib/Socket.h"
#include "frnetlib/Sendable.h"

namespace fr
{
    Socket::Socket()
    : is_blocking(true),
      ai_family(AF_UNSPEC),
      max_receive_size(0),
      socket_read_timeout(0)
    {
        init_wsa();
    }

    Socket::Status Socket::send(const Sendable &obj)
    {
        if(!connected())
            return Socket::Disconnected;

        return obj.send(this);
    }

    Socket::Status Socket::receive(Sendable &obj)
    {
        if(!connected())
            return Socket::Disconnected;

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
            if(status == fr::Socket::Disconnected)
                return status;
            bytes_remaining -= received;
            bytes_read += received;
            if(status == fr::Socket::WouldBlock && bytes_read == 0)
                return status;
        }

        return Socket::Status::Success;
    }

    void Socket::shutdown()
    {
        ::shutdown(get_socket_descriptor(), SHUT_RDWR);
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

    const std::string &Socket::status_to_string(fr::Socket::Status status)
    {
        static std::vector<std::string> map = {
            "Unknown",
            "Success",
            "Listen Failed",
            "Bind Failed",
            "Disconnected",
            "Error",
            "Would Block",
            "Connection Failed",
            "Handshake Failed",
            "Verification Failed",
            "Max packet size exceeded",
            "Not enough data",
            "Parse error",
            "HTTP header too big",
            "HTTP body too big"
        };

        if(status < 0 || status > map.size())
            throw std::logic_error("Socket::status_to_string(): Invalid status value " + std::to_string(status));
        return map[status];
    }

    void Socket::disconnect()
    {
        close_socket();
    }
}