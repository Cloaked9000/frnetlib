//
// Created by fred.nicolson on 02/06/17.
//

#ifndef FRNETLIB_WEBSOCKET_H
#define FRNETLIB_WEBSOCKET_H

#include "Socket.h"
#include "Http.h"

namespace fr
{
    /*!
     * fr::HttpSocket inherits this class. It's sole purpose is
     * to allow you to store fr::HttpSocket<T> types in a generic
     * container.
     */
    class WebSocket
    {
    public:
        virtual ~WebSocket()=default;
        virtual Socket::Status receive(Http &request) = 0;
        virtual Socket::Status send(const Http &request) = 0;
    };
}


#endif //FRNETLIB_WEBSOCKET_H
