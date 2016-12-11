//
// Created by fred on 06/12/16.
//

#include <iostream>
#include "TcpSocket.h"

namespace fr
{

    TcpSocket::TcpSocket() noexcept
    : recv_buffer(new char[RECV_CHUNK_SIZE]),
      is_connected(false)
    {

    }

    TcpSocket::~TcpSocket() noexcept
    {
        close();
    }

    Socket::Status TcpSocket::send(const Packet &packet)
    {
        //Get packet data
        std::string data = packet.get_buffer();

        //Prepend packet length
        uint32_t length = htonl((uint32_t)data.size());
        data.insert(0, "1234");
        memcpy(&data[0], &length, sizeof(uint32_t));

        //Send it
        return send_raw(data.c_str(), data.size());
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
            else
            {
                if(status == -1)
                {
                    return Socket::Status::Error;
                }
                else
                {
                    is_connected = false;
                    return Socket::Status::Disconnected;
                }
            }
        }
        return Socket::Status::Success;
    }

    Socket::Status TcpSocket::receive(Packet &packet)
    {
        Socket::Status status;

        //Try to read packet length
        uint32_t packet_length = 0;
        status = receive_all(&packet_length, sizeof(packet_length));
        if(status != Socket::Status::Success)
            return status;
        packet_length = ntohl(packet_length);

        //Now we've got the length, read the rest of the data in
        std::string data(packet_length, 'c');
        status = receive_all(&data[0], packet_length);
        if(status != Socket::Status::Success)
            return status;

        //Set the packet to what we've read
        packet.set_buffer(std::move(data));

        return Socket::Status::Success;
    }

    void TcpSocket::close()
    {
        if(is_connected)
        {
            ::close(socket_descriptor);
            is_connected = false;
        }
    }

    Socket::Status TcpSocket::receive_all(void *dest, size_t size)
    {
        size_t bytes_read = 0;
        while(bytes_read < size)
        {
            size_t read = 0;
            Socket::Status status = receive_raw((uintptr_t*)dest + bytes_read, size, read);
            if(status == Socket::Status::Success)
                bytes_read += read;
            else
                return status;
        }
        return Socket::Status::Success;
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
                if(status == -1)
                {
                    return Socket::Status::Error;
                }
                else
                {
                    is_connected = false;
                    return Socket::Status::Disconnected;
                }
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

}