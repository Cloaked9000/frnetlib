//
// Created by fred on 20/12/16.
//

#ifndef FRNETLIB_SOCKETREACTOR_H
#define FRNETLIB_SOCKETREACTOR_H

#include <chrono>
#include <functional>
#include <vector>
#include "Socket.h"
#include "SocketSelector.h"

namespace fr
{
    class SocketReactor
    {
    public:
        SocketReactor() noexcept = default;
        SocketReactor(const SocketReactor&)=delete;
        SocketReactor(SocketReactor&&) noexcept = default;

        /*!
         * Adds a socket to the selector. Note that SocketSelector
         * does not keep a copy of the object, just a handle, it's
         * up to you to store your fr::Sockets.
         *
         * @param socket The socket to add.
         * @param callback A function to call when the socket shows activity. Remember to remove it from the reactor if the client has disconnected.
         */
        void add(const Socket &socket, std::function<void()> callback);

        /*!
         * Removes a socket from the socket selector.
         *
         * @param socket The socket to remove.
         */
        void remove(const Socket &socket);

        /*!
         * Waits for a socket to become ready.
         *
         * @param timeout The amount of time wait should block for before timing out.
         * @return True if a socket is ready. False if it timed out.
         */
        bool wait(std::chrono::milliseconds timeout = std::chrono::milliseconds(0));

    private:
        std::vector<std::pair<const fr::Socket*, std::function<void()>>> callbacks;
        fr::SocketSelector socket_selector;

    };
}


#endif //FRNETLIB_SOCKETREACTOR_H
