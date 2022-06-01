#include "los/events.h"
#include "fmt/format.h"
#include "event/io_epoll.h"
#include "event/io_select.h"

namespace los {
namespace events {

std::shared_ptr<IoInterface> CreateIo(int timeout_ms, MultiplexTypes type)
{
    if (MultiplexTypes::kAuto == type)
    {
#if defined(__linux__)
        type = MultiplexTypes::kEpoll;
#else
        type = MultiplexTypes::kSelect;
#endif
    }

    std::shared_ptr<IoInterface> h = nullptr;
    switch (type)
    {
    case los::events::MultiplexTypes::kEpoll:
#if defined(__linux__)
        h = std::make_shared<IoEpoll>(timeout_ms);
#endif
        break;
    case los::events::MultiplexTypes::kSelect:
        h = std::make_shared<IoSelect>(timeout_ms);
        break;
    default:
        break;
    }

    return h;
}

}
}
