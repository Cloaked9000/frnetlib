//
// Created by fred on 06/12/16.
//

#include "TcpSocket.h"

namespace fr
{

    bool TcpSocket::send(const Packet &packet)
    {
        size_t send_index = 0;
        size_t sent = 0;

        while(sent < packet.construct_packet().size())
        {
            ssize_t a = ::send(socket_descriptor, &packet.construct_packet()[send_index], packet.construct_packet().size(), 0);
            if(a < 1)
                return false;
            sent += a;
        }

        return true;
    }

    bool TcpSocket::receive(Packet &packet)
    {
        std::string recv_buffer;

        //Read packet length
        uint32_t packet_length = 0;

        return false;
    }

    void TcpSocket::close()
    {
        ::close(socket_descriptor);
    }

    ssize_t TcpSocket::read_recv()
    {
        return 0;
    }

    void TcpSocket::set_descriptor(int descriptor)
    {
        socket_descriptor = descriptor;
    }
}