//
// Created by fred on 10/12/16.
//

#ifndef FRNETLIB_HTTPSOCKET_H
#define FRNETLIB_HTTPSOCKET_H

#include "Http.h"
#include "Socket.h"
#include "WebSocket.h"

namespace fr
{
    template<class SocketType>
    class HttpSocket : public SocketType, public WebSocket
    {
    public:
        HttpSocket()
        : recv_buffer(HTTP_RECV_BUFFER_SIZE, '\0')
        {

        }

        template<typename T>
        HttpSocket(T &&var)
        : recv_buffer(HTTP_RECV_BUFFER_SIZE, '\0'),
          SocketType(var)
        {

        }

        virtual ~HttpSocket() = default;

        Socket::Status receive(Http &request)
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

        Socket::Status send(const Http &request)
        {
            std::string data = request.construct(SocketType::get_remote_address());
            return SocketType::send_raw(&data[0], data.size());
        }

    private:
        //Create buffer to receive_request the request
        std::string recv_buffer;
    };

}

#endif //FRNETLIB_HTTPSOCKET_H
