#ifndef LOS_INTERNAL_EVENT_IO_EPOLL_H_
#define LOS_INTERNAL_EVENT_IO_EPOLL_H_

#if defined(__linux__)

#include <vector>
#include <unordered_map>
#include <sys/epoll.h>
#include "los/events.h"

namespace los {
namespace events {

struct EpollHandler
{
    HandlerCallback callback;
    void *priv_data;
    int register_events;
};

class IoEpoll : public IIo
{
public:
    IoEpoll() = delete;
    IoEpoll(const IoEpoll &) = delete;
    IoEpoll &operator=(const IoEpoll &) = delete;

    explicit IoEpoll(int timeout_ms);
    virtual ~IoEpoll();

    virtual void RegisterHandler(int fd, HandlerCallback callback, void *priv_data, int register_events);
    virtual void RemoveHandler(int fd);

    virtual void EnableEvent(int fd, int events);
    virtual void DisableEvent(int fd, int events);

    virtual int Execute();

    virtual void SetTimeoutMs(int timeout_ms);

private:
    int timeout_ms_;
    std::unordered_map<int, std::shared_ptr<EpollHandler>> handlers_;

    int epoll_fd_;
    std::vector<epoll_event> epoll_events_;
};

}
}

#endif

#endif // !LOS_INTERNAL_EVENT_IO_EPOLL_H_
