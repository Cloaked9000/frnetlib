//
// Created by fred on 10/12/16.
//

#ifndef FRNETLIB_HTTPSOCKET_H
#define FRNETLIB_HTTPSOCKET_H

#include "TcpSocket.h"
#include "HttpRequest.h"

namespace fr
{
    class HttpSocket : public TcpSocket
    {
    public:
        /*!
         * Sends a HTTP request to the connected socket.
         *
         * @param request The request to send
         * @return The status of the operation.
         */
        Socket::Status receive(HttpRequest &request);

        /*!
         * Sends a HTTP request to the connected socket.
         *
         * @param request Where to store the received request.
         * @return The status of the operation.
         */
        Socket::Status send(const HttpRequest &request);
    };

}

#endif //FRNETLIB_HTTPSOCKET_H
