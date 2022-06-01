#ifdef _WIN32
#include <WinSock2.h>
#else
#include <netinet/in.h>
#endif

#include "event/io_select.h"

namespace los {
namespace events {

IoSelect::IoSelect(int timeout_ms) :
    timeout_ms_(timeout_ms),
    max_fd_(-1)
{
#if defined(_WIN32)
    idle_fd_ = static_cast<int>(socket(AF_INET, SOCK_DGRAM, 0));
    RegisterHandler(idle_fd_, nullptr, nullptr, los::events::kRead);
#endif
}

IoSelect::~IoSelect()
{
#if defined(_WIN32)
    if (idle_fd_ >= 0)
    {
        closesocket(idle_fd_);
        idle_fd_ = -1;
    }
#endif
}

void IoSelect::RegisterHandler(int fd, HandlerCallback callback, void *priv_data, int register_events)
{
    auto handler = std::make_shared<SelectHandler>();
    handler->callback = callback;
    handler->priv_data = priv_data;
    handler->register_events = register_events;

    handlers_.emplace(fd, handler);

    if (fd > max_fd_)
    {
        max_fd_ = fd;
    }
}

void IoSelect::RemoveHandler(int fd)
{
    if (handlers_.end() != handlers_.find(fd))
    {
        handlers_.erase(fd);
    }

    if (fd == max_fd_)
    {
        max_fd_ = 0;
        for (auto &&handler_iter : handlers_)
        {
            if (handler_iter.first > max_fd_)
            {
                max_fd_ = handler_iter.first;
            }
        }
    }
}

void IoSelect::EnableEvent(int fd, int events)
{
    auto iter = handlers_.find(fd);
    if (handlers_.end() != iter)
    {
        iter->second->register_events |= events;
    }
}

void IoSelect::DisableEvent(int fd, int events)
{
    auto iter = handlers_.find(fd);
    if (handlers_.end() != iter)
    {
        iter->second->register_events &= ~events;
    }
}

int IoSelect::Execute()
{
    fd_set rfds, wfds;
    FD_ZERO(&rfds);
    FD_ZERO(&wfds);
    for (auto &&handler_iter : handlers_)
    {
        if (handler_iter.second->register_events & los::events::kRead)
        {
            FD_SET(handler_iter.first, &rfds);
        }

        if (handler_iter.second->register_events & los::events::kWrite)
        {
            FD_SET(handler_iter.first, &wfds);
        }
    }

    timeval timeout_tv = { 0, 1000 * timeout_ms_ };
    int select_ret = select(max_fd_ + 1, &rfds, &wfds, nullptr, &timeout_tv);
    if (select_ret > 0)
    {
        std::unordered_map<int, int> trigger_events;
        for (auto &&handler_iter : handlers_)
        {
            if (FD_ISSET(handler_iter.first, &rfds))
            {
                auto trigger_event_iter = trigger_events.find(handler_iter.first);
                if (trigger_event_iter == trigger_events.end())
                {
                    trigger_events.emplace(handler_iter.first, los::events::kRead);
                }
                else
                {
                    trigger_event_iter->second |= los::events::kRead;
                }
            }

            if (FD_ISSET(handler_iter.first, &wfds))
            {
                auto trigger_event_iter = trigger_events.find(handler_iter.first);
                if (trigger_event_iter == trigger_events.end())
                {
                    trigger_events.emplace(handler_iter.first, los::events::kWrite);
                }
                else
                {
                    trigger_event_iter->second |= los::events::kWrite;
                }
            }
        }

        for (auto &&trigger_event_iter : trigger_events)
        {
            auto handler_iter = handlers_.find(trigger_event_iter.first);
            if (handlers_.end() != handler_iter)
            {
                handler_iter->second->callback(handler_iter->second->priv_data, trigger_event_iter.second);
            }
        }
    }
    else if (EINTR == errno)
    {
        select_ret = 0;
    }

    return select_ret;
}

void IoSelect::SetTimeoutMs(int timeout_ms)
{
    timeout_ms_ = timeout_ms;
}

}
}
