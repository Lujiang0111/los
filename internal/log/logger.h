#ifndef LOS_INTERNAL_LOG_LOGGER_H_
#define LOS_INTERNAL_LOG_LOGGER_H_

#include <stdio.h>
#include <string>
#include <memory>

#include "cores.h"
#include "los/files.h"
#include "los/logs.h"

namespace los {
namespace logs {

struct LogMsg;

class Logger : public LoggerInterface, public std::enable_shared_from_this<Logger>
{
public:
    Logger(const char *path, size_t max_size = 0);
    virtual ~Logger();

    virtual void Log(Levels level, bool is_on_screen, const char *file_name, int file_line, const char *format, ...);
    virtual void LogContent(Levels level, bool is_on_screen, const char *file_name, int file_line, const char *content, size_t content_length);

    void DoLog(size_t id, const LogMsg &msg);
    void DeleteLog(const files::FileInfoInterface *file_info, size_t &del_size);

private:
    std::string path_;
    size_t max_size_;
    int64_t hour_count_;
    FILE *file_;
};

}   // namespace files
}   // namespace los

#endif // !LOS_INTERNAL_LOG_LOGGER_H_
