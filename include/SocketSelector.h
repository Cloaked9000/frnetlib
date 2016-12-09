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

        bool wait();
        void add(const Socket &socket);
        bool is_ready(const Socket &socket);
        void remove(const Socket &socket);
    private:

        fd_set listen_set;
        fd_set listen_read;
        int32_t max_descriptor;
    };
}


#endif //FRNETLIB_SOCKETSELECTOR_H
