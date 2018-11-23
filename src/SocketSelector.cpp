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
        if(!socket->connected())
        {
            throw std::logic_error("Can't add disconnected socket");
        }

        auto add_iter = added_sockets.emplace(socket->get_socket_descriptor(), Opaque(socket->get_socket_descriptor(), socket, opaque));
        if(!add_iter.second)
        {
            throw std::logic_error("Can't add duplicate socket: " + std::to_string(socket->get_socket_descriptor()));
        }

        epoll_event event = {0};
        event.events = EPOLLIN | EPOLLHUP | EPOLLERR | EPOLLRDHUP;
        event.data.ptr = &add_iter.first->second;

        if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket->get_socket_descriptor(), &event) < 0)
        {
            added_sockets.erase(socket->get_socket_descriptor());
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
            if(events[a].events & EPOLLERR || events[a].events & EPOLLHUP || events[a].events & EPOLLRDHUP)
            {
                auto iter = added_sockets.find(opaque->descriptor);
                if(iter != added_sockets.end())
                {
                    epoll_event event = {0};
                    auto remove_ret = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, opaque->descriptor, &event);
                    added_sockets.erase(opaque->descriptor);
                    if(remove_ret < 0)
                    {
                        throw std::runtime_error(
                                "Failed to remove socket: " + std::to_string(opaque->descriptor) + ". Errno: " +
                                std::to_string(errno));
                    }
                }
            }
        }
        return ret;
    }

    void *SocketSelector::remove(const std::shared_ptr<fr::SocketDescriptor> &socket)
    {
        auto descriptor = socket->get_socket_descriptor();
        if(!socket->connected())
        {
            throw std::runtime_error("Can't remove disconnected socket");
        }


        auto iter = added_sockets.find(descriptor);
        if(iter == added_sockets.end())
        {
            return nullptr;
        }

        added_sockets.erase(iter);
        epoll_event event = {0};
        if(epoll_ctl(epoll_fd, EPOLL_CTL_DEL, descriptor, &event) < 0)
        {
            throw std::runtime_error("Failed to remove socket: " + std::to_string(descriptor) + ". Errno: " + std::to_string(errno));
        }
        void *opaque = ((Opaque*)event.data.ptr)->opaque;
        delete static_cast<Opaque *>(event.data.ptr);
        return opaque;
    }

#endif
}

//Windows implementation coming soon(tm)