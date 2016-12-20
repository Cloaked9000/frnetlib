//
// Created by fred on 20/12/16.
//

#include "frnetlib/SocketReactor.h"

namespace fr
{

    void SocketReactor::add(const Socket &socket, std::function<void()> callback)
    {
        socket_selector.add(socket);
    }

    void SocketReactor::remove(const Socket &socket)
    {
        socket_selector.remove(socket);
    }

    bool SocketReactor::wait(std::chrono::milliseconds timeout)
    {
        if(socket_selector.wait(timeout))
        {
            //Find which socket sent the activity
            for(auto &callback : callbacks)
            {
                if(socket_selector.is_ready(callback.first))
                {
                    //Call the socket's callback
                    callback.second();
                }
            }
        }
        return false;
    }
}