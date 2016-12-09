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

    bool SocketSelector::wait()
    {
        listen_read = listen_set;
        if(select(max_descriptor + 1, &listen_read, NULL, NULL, NULL) == SOCKET_ERROR)
        {
            //Check that we've not just timed out
            if(errno == EAGAIN || errno == EWOULDBLOCK)
                return true;

            //Oops
            throw std::logic_error("select() returned -1");
        }

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