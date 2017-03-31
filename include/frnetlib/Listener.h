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

        /*!
         * Calls the shutdown syscall on the socket.
         * So you can receive data but not send.
         *
         * This can be called on a blocking socket to force
         * it to immediately return (you might want to do this if
         * you're exiting and need the blocking socket to return).
         */
        virtual void shutdown()=0;
    };
}


#endif //FRNETLIB_LISTENER_H
