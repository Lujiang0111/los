#if defined(_WIN32)
#else
#include <pthread.h>
#endif

#include "log/log_thread.h"
#include "fmt/format.h"

constexpr size_t kMsgsMaxSize = 1000;

namespace los {
namespace logs {

LogThread &LogThread::GetInstance()
{
    static LogThread instance;
    return instance;
}

LogThread::LogThread() :
    is_msg_full_(false)
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
    EnqueueMsg(msg);

    if (thread_.joinable())
    {
        thread_.join();
    }
}

void LogThread::EnqueueMsg(std::shared_ptr<LogMsg> msg)
{
    {
        std::lock_guard<std::mutex> lock(msg_mutex_);
        msgs_enq_.push_back(msg);
        if (msgs_enq_.size() > kMsgsMaxSize)
        {
            msgs_enq_.clear();
            is_msg_full_ = true;
        }
    }
    msg_cond_.notify_one();
}

bool LogThread::DequeueMsgs()
{
    if (is_msg_full_)
    {
        msgs_deq_.clear();
        is_msg_full_ = false;
    }

    if (msgs_deq_.empty())
    {
        std::unique_lock<std::mutex> lock(msg_mutex_);
        msg_cond_.wait(lock, [this] {return !msgs_enq_.empty(); });
        msgs_enq_.swap(msgs_deq_);
        return true;
    }

    return false;
}

void LogThread::WorkerLoop()
{
    size_t id = 0;
    bool loop_running = true;
    while (loop_running)
    {
        DequeueMsgs();

        LogMsg *msg = msgs_deq_.front().get();
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

        msgs_deq_.pop_front();
    }
}

}   // namespace logs
}   // namespace los
