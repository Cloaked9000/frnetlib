//
// Created by fred on 09/12/16.
//

#include "SocketSelector.h"

namespace fr
{

    SocketSelector::SocketSelector() noexcept
    {
        //Zero out sets
        FD_ZERO(&listen_set);
        FD_ZERO(&listen_read);

        max_descriptor = 0;
    }

    void SocketSelector::add(const Socket &socket)
    {
        //Add it to the set
        FD_SET(socket.get_socket_descriptor(), &listen_set);

        if(socket.get_socket_descriptor() > max_descriptor)
            max_descriptor = socket.get_socket_descriptor();
    }

    bool SocketSelector::wait(std::chrono::milliseconds timeout)
    {
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

    void SocketSelector::remove(const Socket &socket)
    {
        FD_CLR(socket.get_socket_descriptor(), &listen_set);
    }

    bool SocketSelector::is_ready(const Socket &socket)
    {
        return (FD_ISSET(socket.get_socket_descriptor(), &listen_read));
    }
}