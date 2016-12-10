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
        Socket::Status receive_request(HttpRequest &request);

        /*!
         * Sends a HTTP response to the connected socket.
         *
         * @param request The response to send
         * @return The status of the operation.
         */
        Socket::Status receive_response(HttpRequest &response);

        /*!
         * Sends a HTTP request to the connected socket.
         *
         * @param request The HTTP request to send.
         * @return The status of the operation.
         */
        Socket::Status send_request(const HttpRequest &request);

        /*!
         * Sends a HTTP response to the connected socket.
         *
         * @param request The HTTP response to send.
         * @return The status of the operation.
         */
        Socket::Status send_response(const HttpRequest &request);
    };

}

#endif //FRNETLIB_HTTPSOCKET_H
