#ifndef LOS_INTERNAL_LOG_LOG_THREAD_H_
#define LOS_INTERNAL_LOG_LOG_THREAD_H_

#include <thread>
#include <condition_variable>

#include "log/circular_q.h"
#include "log/logger.h"

namespace los {
namespace logs {

enum MsgTypes
{
    kLog = 0,
    kPrint,
    kPrintln,
    kTerminate,
};

struct LogMsg
{
    std::shared_ptr<Logger> logger;
    MsgTypes type{ MsgTypes::kLog };
    std::chrono::system_clock::time_point time;
    size_t thread_id{ 0 };
    Levels level{ Levels::kDebug };
    bool print_screen{ false };
    std::string name;
    int line{ 0 };
    std::string content;
};

class LogThread
{
public:
    LogThread(const LogThread &) = delete;
    LogThread &operator=(const LogThread &) = delete;
    virtual ~LogThread();

    static LogThread &GetInstance();

    void Enqueue(LogMsg &&msg);
    bool DequeueFor(LogMsg &msg, size_t wait_ms);

private:
    LogThread();

    void WorkerLoop();

private:
    std::thread thread_;
    std::mutex msgs_mutex_;
    std::condition_variable push_cv_;
    circular_q<LogMsg> msgs_;
};

}   // namespace files
}   // namespace los

#endif // !LOS_INTERNAL_LOG_LOG_THREAD_H_
