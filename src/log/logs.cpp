#include "log/log_thread.h"

namespace los {
namespace logs {

std::shared_ptr<LoggerInterface> CreateLogger(const char *path, size_t max_size)
{
    return std::make_shared<Logger>(Logger(path, max_size));
}

LoggerInterface *DefaultLogger()
{
    static auto default_logger = CreateLogger("Log", 0);
    return default_logger.get();
}

void Printf(const char *format, ...)
{
    std::vector<char> content_buf(256);
    va_list vl;
    va_start(vl, format);
    int content_len = VsprintfForVector(content_buf, format, vl);
    va_end(vl);

    if (content_len <= 0)
    {
        return;
    }

    std::shared_ptr<LogMsg> msg = std::make_shared<LogMsg>();
    msg->type = kPrint;
    msg->content = &content_buf[0];

    msg->promise = std::make_shared<std::promise<bool>>();
    auto fut = msg->promise->get_future();
    LogThread::GetInstance().EnqueueMsg(msg);
    fut.get();
}

void Printfln(const char *format, ...)
{
    std::vector<char> content_buf(256);
    va_list vl;
    va_start(vl, format);
    int content_len = VsprintfForVector(content_buf, format, vl);
    va_end(vl);

    if (content_len <= 0)
    {
        return;
    }

    std::shared_ptr<LogMsg> msg = std::make_shared<LogMsg>();
    msg->type = kPrintln;
    msg->content = &content_buf[0];

    msg->promise = std::make_shared<std::promise<bool>>();
    auto fut = msg->promise->get_future();
    LogThread::GetInstance().EnqueueMsg(msg);
    fut.get();
}

}   // namespace logs
}   // namespace los
