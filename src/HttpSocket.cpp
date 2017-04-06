//
// Created by fred on 10/12/16.
//

#include <frnetlib/HttpResponse.h>
#include "frnetlib/HttpSocket.h"

namespace fr
{
    template<class SocketType>
    HttpSocket<SocketType>::HttpSocket()
    : recv_buffer(RECV_CHUNK_SIZE, '\0')
    {

    }

    template<class SocketType>
    template<typename T>
    HttpSocket<SocketType>::HttpSocket(T &&var)
    : recv_buffer(RECV_CHUNK_SIZE, '\0'),
      SocketType(var)
    {

    }

    template<class SocketType>
    Socket::Status HttpSocket<SocketType>::receive(Http &request)
    {
        size_t received = 0;
        do
        {
            //Receive the request
            Socket::Status status = SocketType::receive_raw(&recv_buffer[0], recv_buffer.size(), received);
            if(status != Socket::Success)
                return status;
            recv_buffer.resize(received);

            //Parse it
        } while(request.parse(recv_buffer.substr(0, received)));

        return Socket::Success;
    }

    template<class SocketType>
    Socket::Status HttpSocket<SocketType>::send(const Http &request)
    {
        std::string data = request.construct(SocketType::get_remote_address());
        return SocketType::send_raw(&data[0], data.size());
    }
}