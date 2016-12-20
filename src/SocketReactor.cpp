//
// Created by fred on 20/12/16.
//

#include <algorithm>
#include "frnetlib/SocketReactor.h"

namespace fr
{

    void SocketReactor::add(const Socket &socket, std::function<void()> callback)
    {
        socket_selector.add(socket);
        callbacks.emplace_back(std::make_pair(&socket, callback));
    }

    void SocketReactor::remove(const Socket &socket)
    {
        socket_selector.remove(socket);
        callbacks.erase(std::find_if(callbacks.begin(), callbacks.end(), [&](const std::pair<const fr::Socket*, std::function<void()>> &check) {
            return check.first->get_socket_descriptor() == socket.get_socket_descriptor();
        }));
    }

    bool SocketReactor::wait(std::chrono::milliseconds timeout)
    {
        bool found = false;
        if(socket_selector.wait(timeout))
        {
            //Find which socket sent the activity
            for(auto &callback : callbacks)
            {
                if(socket_selector.is_ready(*callback.first))
                {
                    //Call the socket's callback
                    callback.second();
                    found = true;
                }
            }
        }
        return found;
    }
}