//
// Created by fred on 10/12/16.
//

#ifndef FRNETLIB_HTTPSOCKET_H
#define FRNETLIB_HTTPSOCKET_H

#include "TcpSocket.h"
#include "Http.h"
#include "SSLContext.h"

namespace fr
{
    template<class SocketType>
    class HttpSocket : public SocketType
    {
    public:
        //Constructor
        HttpSocket();

        //Forward constructor arguments to SocketType if needed
        template<typename T>
        HttpSocket(T &&var);

        /*!
         * Receives a HTTP request from the connected socket
         *
         * @param request Where to store the received request.
         * @return The status of the operation.
         */
        Socket::Status receive(Http &request);

        /*!
         * Sends a HTTP request to the connected socket.
         *
         * @param request The HTTP request to send.
         * @return The status of the operation.
         */
        Socket::Status send(const Http &request);

    private:
        //Create buffer to receive_request the request
        std::string recv_buffer;
    };

}

#endif //FRNETLIB_HTTPSOCKET_H
