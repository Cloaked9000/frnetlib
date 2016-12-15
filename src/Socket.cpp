//
// Created by fred on 06/12/16.
//

#include "Socket.h"

namespace fr
{

    Socket::Socket() noexcept
    : is_blocking(true),
      is_connected(false)
    {

    }

    Socket::Status Socket::send(const Packet &packet)
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

    Socket::Status Socket::receive(Packet &packet)
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

    Socket::Status Socket::receive_all(void *dest, size_t size)
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
}