#if defined(_WIN32)
#else
#include <pthread.h>
#endif

#include "log/log_thread.h"
#include "fmt/format.h"

constexpr size_t kMaxItems = 1000;

namespace los {
namespace logs {

LogThread &LogThread::GetInstance()
{
    static LogThread instance;
    return instance;
}

LogThread::LogThread() : msgs_(kMaxItems)
{
    thread_ = std::thread(&LogThread::WorkerLoop, this);
#if defined(_WIN32)
#else
    pthread_setname_np(thread_.native_handle(), "log");
#endif
}

LogThread::~LogThread()
{
    // post a kTerminate msg
    std::shared_ptr<LogMsg> msg = std::make_shared<LogMsg>();
    msg->type = kTerminate;
    Enqueue(msg);

    if (thread_.joinable())
    {
        thread_.join();
    }
}

void LogThread::Enqueue(std::shared_ptr<LogMsg> msg)
{
    {
        std::unique_lock<std::mutex> lock(msgs_mutex_);
        msgs_.push_back(std::move(msg));
    }
    push_cv_.notify_one();
}

std::shared_ptr<LogMsg> LogThread::DequeueFor(size_t wait_ms)
{
    std::shared_ptr<LogMsg> msg = nullptr;
    {
        std::unique_lock<std::mutex> lock(msgs_mutex_);
        if (!push_cv_.wait_for(lock, std::chrono::milliseconds(wait_ms), [this] { return !this->msgs_.empty(); }))
        {
            return false;
        }
        msg = msgs_.front();
        msgs_.pop_front();
    }
    return msg;
}

void LogThread::WorkerLoop()
{
    size_t id = 0;
    bool loop_running = true;
    while (loop_running)
    {
        auto msg = DequeueFor(10000);
        if (!msg)
        {
            continue;
        }

        switch (msg->type)
        {
        case kLog:
            msg->logger->DoLog(id++, *msg);
            break;
        case kPrint:
            fmt::print("{}", msg->content);
            break;
        case kPrintln:
            fmt::print("{}\n", msg->content);
            break;
        case kTerminate:
            loop_running = false;
            break;
        default:
            break;
        }

        if (msg->promise)
        {
            msg->promise->set_value(true);
        }
    }
}

}   // namespace logs
}   // namespace los
