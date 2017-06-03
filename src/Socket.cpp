//
// Created by fred on 06/12/16.
//

#include <mutex>
#include <csignal>
#include "frnetlib/NetworkEncoding.h"
#include "frnetlib/Socket.h"

namespace fr
{
    #ifdef _WIN32
        WSADATA Socket::wsaData = WSADATA();
    #endif // _WIN32
    uint32_t Socket::instance_count = 0;

    Socket::Socket() noexcept
    : is_blocking(true),
      ai_family(AF_UNSPEC),
      max_packet_size(0)
    {
            if(instance_count == 0)
            {
                #ifdef _WIN32
                    int wsa_result = WSAStartup(MAKEWORD(2, 2), &wsaData);
                    if(wsa_result != 0)
                    {
                        std::cout << "Failed to initialise WSA." << std::endl;
                        return;
                    }
                #else
                    //Disable SIGPIPE
                    signal(SIGPIPE, SIG_IGN);
                #endif // _WIN32
            }
			instance_count++;
    }

    Socket::Status Socket::send(Packet &packet)
    {
        if(!connected())
            return Socket::Disconnected;

        std::string &data = packet.get_buffer();
        return send_raw(data.c_str(), data.size());
    }

    Socket::Status Socket::send(Packet &&packet)
    {
        if(!connected())
            return Socket::Disconnected;

        std::string &data = packet.get_buffer();
        return send_raw(data.c_str(), data.size());
    }

    Socket::Status Socket::receive(Packet &packet)
    {
        if(!connected())
            return Socket::Disconnected;

        Socket::Status status;
        std::lock_guard<std::mutex> guard(inbound_mutex);

        //Try to read packet length
        uint32_t packet_length = 0;
        status = receive_all(&packet_length, sizeof(packet_length));
        if(status != Socket::Status::Success)
            return status;
        packet_length = ntohl(packet_length);

        //Check that packet_length doesn't exceed the limit, if any
        if(max_packet_size && max_packet_size > max_packet_size)
        {
            close_socket();
            return fr::Socket::MaxPacketSizeExceeded;
        }

        //Now we've got the length, read the rest of the data in
        packet.buffer.resize(packet_length + PACKET_HEADER_LENGTH);
        status = receive_all(&packet.buffer[PACKET_HEADER_LENGTH], packet_length);
        if(status != Socket::Status::Success)
            return status;


        return Socket::Status::Success;
    }

    Socket::Status Socket::receive_all(void *dest, size_t buffer_size)
    {
        if(!connected())
            return Socket::Disconnected;

		int32_t bytes_remaining = (int32_t) buffer_size;
        size_t bytes_read = 0;
        while(bytes_remaining > 0)
        {
            size_t received = 0;
            char *arr = (char*)dest;
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

    void Socket::set_max_packet_size(uint32_t sz)
    {
        max_packet_size = sz;
    }
}