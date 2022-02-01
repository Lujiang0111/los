﻿#include "log/log_thread.h"

namespace los {
namespace logs {

std::shared_ptr<LoggerInterface> CreateLogger(const char *path, size_t max_size)
{
    return std::make_shared<Logger>(Logger(path, max_size));
}

LoggerInterface *DefaultLogger()
{
    static Logger default_logger = Logger("Log");
    return &default_logger;
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

    LogMsg msg;
    msg.type = kPrint;
    msg.content = &content_buf[0];
    LogThread::GetInstance().Enqueue(std::move(msg));
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

    LogMsg msg;
    msg.type = kPrintln;
    msg.content = &content_buf[0];
    LogThread::GetInstance().Enqueue(std::move(msg));
}

}   // namespace logs
}   // namespace los