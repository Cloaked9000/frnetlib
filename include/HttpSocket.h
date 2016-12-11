//
// Created by fred on 10/12/16.
//

#ifndef FRNETLIB_HTTPSOCKET_H
#define FRNETLIB_HTTPSOCKET_H

#include "TcpSocket.h"
#include "Http.h"

namespace fr
{
    class HttpSocket : public TcpSocket
    {
    public:
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
    };

}

#endif //FRNETLIB_HTTPSOCKET_H
