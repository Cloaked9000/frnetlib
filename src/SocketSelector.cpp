//
// Created by fred on 09/12/16.
//

#include <thread>
#include <mutex>
#include "frnetlib/SocketSelector.h"

//Linux EPOLL implementation
namespace fr
{
#ifndef _WIN32

    SocketSelector::SocketSelector()
    : epoll_fd(-1)
    {
        epoll_fd = epoll_create1(O_CLOEXEC);
        if(epoll_fd < 0)
        {
            throw std::runtime_error("Failed to create EPOLL descriptor: " + std::to_string(errno));
        }
    }

    SocketSelector::~SocketSelector()
    {
        close(epoll_fd);
    }

    void SocketSelector::add(const std::shared_ptr<fr::SocketDescriptor> &socket, void *opaque)
    {
        int32_t descriptor = socket->get_socket_descriptor();
        if(!socket->connected())
        {
            throw std::logic_error("Can't add disconnected socket");
        }

        auto added_iter = added_sockets.emplace((uintptr_t)socket.get(), Opaque(socket, opaque, descriptor));
        if(!added_iter.second)
        {
            throw std::logic_error("Can't add duplicate socket");
        }

        epoll_event event = {0};
        event.events = EPOLLIN | EPOLLHUP | EPOLLERR | EPOLLRDHUP;
        event.data.ptr = &added_iter.first->second;

        if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, descriptor, &event) < 0)
        {
            delete (Opaque*)event.data.ptr;
            throw std::runtime_error("Failed to add socket: " + std::to_string(errno));
        }
    }

    std::vector<std::pair<std::shared_ptr<fr::SocketDescriptor>, void *>>
    SocketSelector::wait(std::chrono::milliseconds timeout)
    {
        static thread_local epoll_event events[100];
        int event_count = epoll_wait(epoll_fd, events, 100, timeout.count());
        if(event_count < 0)
        {
            if(errno == EINTR)
            {
                return {};
            }
            throw std::runtime_error("epoll_wait returned: " + std::to_string(errno));
        }

        std::vector<std::pair<std::shared_ptr<fr::SocketDescriptor>, void *>> ret;
        for(int a = 0; a < event_count; ++a)
        {
            auto *opaque = static_cast<Opaque *>(events[a].data.ptr);
            ret.emplace_back(opaque->socket, opaque->opaque);
        }
        return ret;
    }

    void *SocketSelector::remove(const std::shared_ptr<fr::SocketDescriptor> &socket)
    {
        auto iter = added_sockets.find((uintptr_t)socket.get());
        if(iter == added_sockets.end())
        {
            return nullptr;
        }

        if(epoll_ctl(epoll_fd, EPOLL_CTL_DEL, iter->second.descriptor, nullptr) < 0)
        {
            throw std::runtime_error("Failed to remove socket: " + std::to_string(iter->second.descriptor) + ". Errno: " + std::to_string(errno));
        }

        void *opaque = iter->second.opaque;
        added_sockets.erase(iter);
        return opaque;
    }

#endif
}

//Windows implementation coming soon(tm)