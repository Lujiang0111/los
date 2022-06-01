#if defined(__linux__)

#include "event/io_epoll.h"
#include <unistd.h>

namespace los {
namespace events {

IoEpoll::IoEpoll(int timeout_ms) :
    timeout_ms_(timeout_ms),
    epoll_events_(1)
{
    epoll_fd_ = epoll_create1(0);
}

IoEpoll::~IoEpoll()
{
    if (epoll_fd_ >= 0)
    {
        close(epoll_fd_);
        epoll_fd_ = -1;
    }
}

void IoEpoll::RegisterHandler(int fd, HandlerCallback callback, void *priv_data, int register_events)
{
    auto handler = std::make_shared<EpollHandler>();
    handler->callback = callback;
    handler->priv_data = priv_data;
    handler->register_events = register_events;

    if (handlers_.end() != handlers_.find(fd))
    {
        epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
    }
    handlers_.emplace(fd, handler);
    if (handlers_.size() > epoll_events_.size())
    {
        epoll_events_.resize(handlers_.size() + 16);
    }

    epoll_event ev = { 0 };
    if (register_events & los::events::kRead)
    {
        ev.events |= EPOLLIN;
    }
    if (register_events & los::events::kWrite)
    {
        ev.events |= EPOLLOUT;
    }
    ev.data.fd = fd;
    epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev);
}

void IoEpoll::RemoveHandler(int fd)
{
    if (handlers_.end() != handlers_.find(fd))
    {
        epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
    }
    handlers_.erase(fd);
}

void IoEpoll::EnableEvent(int fd, int events)
{
    auto iter = handlers_.find(fd);
    if (handlers_.end() != iter)
    {
        iter->second->register_events |= events;

        epoll_event ev = { 0 };
        if (iter->second->register_events & los::events::kRead)
        {
            ev.events |= EPOLLIN;
        }
        if (iter->second->register_events & los::events::kWrite)
        {
            ev.events |= EPOLLOUT;
        }
        ev.data.fd = fd;
        epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev);
    }
}

void IoEpoll::DisableEvent(int fd, int events)
{
    auto iter = handlers_.find(fd);
    if (handlers_.end() != iter)
    {
        iter->second->register_events &= ~events;

        epoll_event ev = { 0 };
        if (iter->second->register_events & los::events::kRead)
        {
            ev.events |= EPOLLIN;
        }
        if (iter->second->register_events & los::events::kWrite)
        {
            ev.events |= EPOLLOUT;
        }
        ev.data.fd = fd;
        epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev);
    }
}

int IoEpoll::Execute()
{
    int nfds = epoll_wait(epoll_fd_, &epoll_events_[0], static_cast<int>(epoll_events_.size()), timeout_ms_);
    if (nfds > 0)
    {
        for (int i = 0; i < nfds; ++i)
        {
            int event_type = 0;
            if (epoll_events_[i].events & EPOLLIN)
            {
                event_type |= los::events::kRead;
            }
            if (epoll_events_[i].events & EPOLLOUT)
            {
                event_type |= los::events::kWrite;
            }

            auto handler_iter = handlers_.find(epoll_events_[i].data.fd);
            if (handlers_.end() != handler_iter)
            {
                handler_iter->second->callback(handler_iter->second->priv_data, event_type);
            }
        }
    }
    else if (EINTR == errno)
    {
        nfds = 0;
    }

    return nfds;
}

void IoEpoll::SetTimeoutMs(int timeout_ms)
{
    timeout_ms_ = timeout_ms;
}

}
}

#endif
