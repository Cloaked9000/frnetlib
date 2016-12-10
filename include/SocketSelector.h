//
// Created by fred on 09/12/16.
//

#ifndef FRNETLIB_SOCKETSELECTOR_H
#define FRNETLIB_SOCKETSELECTOR_H

#include "NetworkEncoding.h"
#include "Socket.h"
#include "TcpListener.h"

namespace fr
{
    class SocketSelector
    {
    public:
        SocketSelector() noexcept;

        /*!
         * Waits for a socket to become ready.
         *
         * @return True if a socket is ready. False if it timed out.
         */
        bool wait();

        /*!
         * Adds a socket to the selector. Note that SocketSelector
         * does not keep a copy of the object, just a handle, it's
         * up to you to store your fr::Sockets.
         *
         * @param socket The socket to add.
         */
        void add(const Socket &socket);

        /*!
         * Checks to see if a socket inside of the selector is ready.
         * This should be called after 'wait' returns true, on
         * each of the added sockets to see which one has received data.
         *
         * @param socket The socket to check if it's ready
         * @return True if this socket is ready, false otherwise.
         */
        bool is_ready(const Socket &socket);

        /*!
         * Removes a socket from the socket selector.
         *
         * @param socket The socket to remove.
         */
        void remove(const Socket &socket);
    private:

        fd_set listen_set;
        fd_set listen_read;
        int32_t max_descriptor;
    };
}


#endif //FRNETLIB_SOCKETSELECTOR_H
