//
// Created by fred.nicolson on 3/31/17.
//

#ifndef FRNETLIB_LISTENER_H
#define FRNETLIB_LISTENER_H
#include "Socket.h"

namespace fr
{
    class Listener
    {
    public:
        /*!
         * Listens to the given port for connections
         *
         * @param port The port to bind to
         * @return If the operation was successful
         */
        virtual Socket::Status listen(const std::string &port)=0;

        /*!
         * Accepts a new connection.
         *
         * @param client Where to store the connection information
         * @return True on success. False on failure.
         */
        virtual Socket::Status accept(Socket &client)=0;
    };
}


#endif //FRNETLIB_LISTENER_H
