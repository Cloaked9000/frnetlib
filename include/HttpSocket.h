//
// Created by fred on 10/12/16.
//

#ifndef FRNETLIB_HTTPSOCKET_H
#define FRNETLIB_HTTPSOCKET_H

#include "TcpSocket.h"
#include "Http.h"

namespace fr
{
    template<class SocketType>
    class HttpSocket : public SocketType
    {
    public:
        /*!
         * Receives a HTTP request from the connected socket
         *
         * @param request Where to store the received request.
         * @return The status of the operation.
         */
        Socket::Status receive(Http &request)
        {
            //Create buffer to receive_request the request
            std::string buffer(2048, '\0');

            //Receive the request
            size_t received;
            Socket::Status status = SocketType::receive_raw(&buffer[0], buffer.size(), received);
            if(status != Socket::Success)
                return status;
            buffer.resize(received);

            //Parse it
            request.parse(buffer);

            return Socket::Success;
        }

        /*!
         * Sends a HTTP request to the connected socket.
         *
         * @param request The HTTP request to send.
         * @return The status of the operation.
         */
        Socket::Status send(const Http &request)
        {
            std::string data = request.construct(SocketType::get_remote_address());
            return SocketType::send_raw(&data[0], data.size());
        }
    };

}

#endif //FRNETLIB_HTTPSOCKET_H
