//
// Created by fred on 09/12/16.
//

#ifndef FRNETLIB_SOCKETSELECTOR_H
#define FRNETLIB_SOCKETSELECTOR_H

#include <chrono>
#include <vector>
#include <iostream>
#include <unordered_map>
#include "NetworkEncoding.h"
#include "Socket.h"
#include "TcpListener.h"

namespace fr
{
    class SocketSelector
    {
    public:
        SocketSelector();
        ~SocketSelector();

        /*!
         * Adds a socket to the selector along with some opaque state
         *
         * @throws An std::exception on failure
         * @param socket The socket to add, can be a Listener/Socket.
         * @param opaque Opaque data which is passed back by wait() when the socket
         * has activity. Can be used for state management.
         */
        void add(const std::shared_ptr<fr::SocketDescriptor> &socket, void *opaque);

        /*!
         * Waits for activity on one of the added sockets. If a socket disconnects,
         * then it will automatically be removed from the selector, and so remove()
         * should not be called.
         *
         * @throws An std::exception on failure
         * @param timeout The maximum time in milliseconds to wait for. Default/-1 for no timeout.
         * @return A list of sockets which either are ready, or have disconnected. This can be empty
         * if there is a timeout, or the wait is interrupted.
         */
        std::vector<std::pair<std::shared_ptr<fr::SocketDescriptor>, void*>> wait(std::chrono::milliseconds timeout = std::chrono::milliseconds(-1));

        /*!
         * Removes a socket from the selector.
         * Does nothing if the socket isn't a member.
         *
         * @throws An std::exception if an internal EPOLL error occurs
         * @param socket The socket to remove. May have been disconnected.
         * @return The opaque data passed to add(). Or nullptr if the socket wasn't found.
         */
        void *remove(const std::shared_ptr<fr::SocketDescriptor> &socket);
    private:

#ifndef _WIN32
        struct Opaque
        {
            Opaque(std::shared_ptr<fr::SocketDescriptor> socket_, void *opaque_, int32_t descriptor_)
            : socket(std::move(socket_)),
              opaque(opaque_),
              descriptor(descriptor_)
            {}

            std::shared_ptr<fr::SocketDescriptor> socket;
            void *opaque;
            int32_t descriptor;
        };
        int epoll_fd;
        std::unordered_map<uintptr_t, Opaque> added_sockets;
#endif
    };
}


#endif //FRNETLIB_SOCKETSELECTOR_H
