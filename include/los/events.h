#ifndef LOS_INCLUDE_LOS_EVENTS_H_
#define LOS_INCLUDE_LOS_EVENTS_H_

#include "los.h"

namespace los {
namespace events {

enum class MultiplexTypes : int
{
    kAuto = 0,
    kEpoll,         // only for linux
    kSelect,
};

enum EventTypes : int
{
    kRead = 0x01,
    kWrite = 0x02,
};

typedef void (*HandlerCallback)(void *priv_data, int trigger_events);

class LOS_API IoInterface
{
public:
    virtual ~IoInterface() = default;

    virtual void RegisterHandler(int fd, HandlerCallback callback, void *priv_data, int register_events) = 0;
    virtual void RemoveHandler(int fd) = 0;

    virtual void EnableEvent(int fd, int events) = 0;
    virtual void DisableEvent(int fd, int events) = 0;

    virtual int Execute() = 0;

    virtual void SetTimeoutMs(int timeout_ms) = 0;
};

LOS_API std::shared_ptr<IoInterface> CreateIo(int timeout_ms, MultiplexTypes type);

}
}

#endif // !LOS_INCLUDE_LOS_EVENTS_H_
