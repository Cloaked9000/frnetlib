//
// Created by fred on 09/12/16.
//

#include <thread>
#include <mutex>
#include "frnetlib/SocketSelector.h"

namespace fr
{

    SocketSelector::SocketSelector() noexcept
    {
        //Zero out sets
        FD_ZERO(&listen_set);
        FD_ZERO(&listen_read);

        max_descriptor = 0;
    }

    bool SocketSelector::wait(std::chrono::milliseconds timeout)
    {
        //Windows will crash if we pass an empty set. Do a check.
#ifdef _WIN32
        if(listen_set.fd_count == 0)
        {
            //It's empty. Emulate UNIX behaviour by sleeping for timeout.
            std::this_thread::sleep_for(timeout);
            return false;
        }
#endif

        timeval wait_time;
        wait_time.tv_sec = 0;
        wait_time.tv_usec = std::chrono::duration_cast<std::chrono::microseconds>(timeout).count();

        listen_read = listen_set;
        int select_result = select(max_descriptor + 1, &listen_read, NULL, NULL, timeout == std::chrono::milliseconds(0) ? NULL : &wait_time);

        if(select_result == 0) //If it's timed out
            return false;
        else if(select_result == SOCKET_ERROR) //Else if error
            throw std::logic_error("select() returned -1. Errno: " + std::to_string(errno));

        return true;
    }
}