//
// Created by fred on 20/12/16.
//

#ifndef FRNETLIB_SOCKETREACTOR_H
#define FRNETLIB_SOCKETREACTOR_H

#include <chrono>
#include <functional>
#include <algorithm>
#include <vector>
#include "Socket.h"
#include "SocketSelector.h"

namespace fr
{
    class SocketReactor
    {
    public:
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
        template<typename T>
        inline void add(const T &socket, std::function<void()> callback)
        {
            socket_selector.add(socket);
            socket_callbacks.emplace_back(std::make_pair(socket.get_remote_address(), callback));
        }

        /*!
         * Removes a socket from the socket selector.
         *
         * @param socket The socket to remove.
         */
        template<typename T>
        inline void remove(const T &socket)
        {
            socket_selector.remove(socket);
            socket_callbacks.erase(std::find_if(socket_callbacks.begin(), socket_callbacks.end(), [&](const std::pair<const int32_t, std::function<void()>> &check) {
                return check.first == socket.get_socket_descriptor();
            }));
        }

        /*!
         * Waits for a socket to become ready.
         *
         * @param timeout The amount of time wait should block for before timing out.
         * @return True if a socket is ready. False if it timed out.
         */
        bool wait(std::chrono::milliseconds timeout = std::chrono::milliseconds(0));

    private:
        std::vector<std::pair<const int32_t, std::function<void()>>> socket_callbacks; //<descriptor, callback>
        fr::SocketSelector socket_selector;

    };
}


#endif //FRNETLIB_SOCKETREACTOR_H
