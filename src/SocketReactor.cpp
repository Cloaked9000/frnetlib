//
// Created by fred on 20/12/16.
//

#include <algorithm>
#include "frnetlib/SocketReactor.h"

namespace fr
{
    bool SocketReactor::wait(std::chrono::milliseconds timeout)
    {
        bool found = false;
        if(socket_selector.wait(timeout))
        {
            //Find which socket sent the activity
            for(auto &callback : socket_callbacks)
            {
                if(socket_selector.is_ready(callback.first))
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