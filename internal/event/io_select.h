#ifndef LOS_INTERNAL_EVENT_IO_SELECT_H_
#define LOS_INTERNAL_EVENT_IO_SELECT_H_

#include <unordered_map>
#include "los/events.h"

namespace los {
namespace events {

struct SelectHandler
{
    HandlerCallback callback;
    void *priv_data;
    int register_events;
};

class IoSelect : public IIo
{
public:
    IoSelect() = delete;
    IoSelect(const IoSelect &) = delete;
    IoSelect &operator=(const IoSelect &) = delete;

    explicit IoSelect(int timeout_ms);
    virtual ~IoSelect();

    virtual void RegisterHandler(int fd, HandlerCallback callback, void *priv_data, int register_events);
    virtual void RemoveHandler(int fd);

    virtual void EnableEvent(int fd, int events);
    virtual void DisableEvent(int fd, int events);

    virtual int Execute();

    virtual void SetTimeoutMs(int timeout_ms);

private:
    int timeout_ms_;

    int max_fd_;
    std::unordered_map<int, std::shared_ptr<SelectHandler>> handlers_;

#if defined(_WIN32)
    int idle_fd_;
#endif
};

}
}

#endif // !LOS_INTERNAL_EVENT_IO_SELECT_H_
