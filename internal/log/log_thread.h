#ifndef LOS_INTERNAL_LOG_LOG_THREAD_H_
#define LOS_INTERNAL_LOG_LOG_THREAD_H_

#include <deque>
#include <memory>
#include <future>
#include <thread>
#include <condition_variable>

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
    MsgTypes type;
    std::chrono::system_clock::time_point time;
    size_t thread_id;
    Levels level;
    bool is_on_screen;
    std::string file_name;
    int file_line;
    std::string content;
    std::shared_ptr<std::promise<bool>> promise;
};

class LogThread
{
public:
    LogThread(const LogThread &) = delete;
    LogThread &operator=(const LogThread &) = delete;
    virtual ~LogThread();

    static LogThread &GetInstance();

    /***************************************************************************//**
     * 消息入队列
     * @param   path        [in]    消息句柄
     ******************************************************************************/
    void EnqueueMsg(std::shared_ptr<LogMsg> msg);

    /***************************************************************************//**
     * 消息出队列
     * @return  是否有新的消息从msgs_enq_进入msgs_deq_队列
     ******************************************************************************/
    bool DequeueMsgs();

private:
    LogThread();

    void WorkerLoop();

private:
    std::thread thread_;

    std::deque<std::shared_ptr<LogMsg>> msgs_enq_;          // msg入队列
    std::deque<std::shared_ptr<LogMsg>> msgs_deq_;          // msg出队列
    std::mutex msg_mutex_;
    std::condition_variable msg_cond_;
    bool is_msg_full_;
};

}   // namespace files
}   // namespace los

#endif // !LOS_INTERNAL_LOG_LOG_THREAD_H_
