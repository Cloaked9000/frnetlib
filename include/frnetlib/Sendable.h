//
// Created by fred.nicolson on 18/07/17.
//

#ifndef FRNETLIB_SENDABLE_H
#define FRNETLIB_SENDABLE_H
#include "Socket.h"

namespace fr
{
    class Sendable
    {
    public:
        /*!
         * Overridable send, to allow
         * custom types to be directly sent through
         * sockets.
         *
         * @param socket The socket to send through
         * @return Status indicating if the send succeeded or not.
         */
        virtual Socket::Status send(Socket *socket) = 0; //TODO: RETURN PROPER VALUE FROM HTTP PARSE

        /*!
         * Overrideable receive, to allow
         * custom types to be directly received through
         * sockets.
         *
         * @param socket The socket to send through
         * @return Status indicating if the send succeeded or not.
         */
        virtual Socket::Status receive(Socket *socket) = 0;
    };
}


#endif //FRNETLIB_SENDABLE_H
