#ifndef LOS_INTERNAL_LOG_LOGGER_H_
#define LOS_INTERNAL_LOG_LOGGER_H_

#include <string>
#include <memory>

#include "cores.h"
#include "los/files.h"
#include "los/logs.h"
#include "fmt/os.h"

namespace los {
namespace logs {

struct LogMsg;

class Logger : public LoggerInterface, public std::enable_shared_from_this<Logger>
{
public:
    Logger(const char *path, size_t max_size = 0);
    virtual ~Logger() = default;

    virtual void Log(Levels level, bool print_screen, const char *name, int line, const char *format, ...);
    virtual void LogContent(Levels level, bool print_screen, const char *name, int line, const char *content, size_t content_length);

    void DoLog(size_t id, const LogMsg &msg);
    void DeleteLog(const files::FileInfoInterface *file_info, size_t &del_size);

private:
    std::string path_;
    size_t max_size_;
    int64_t hour_count_;
    std::shared_ptr<fmt::ostream> file_;
};

}   // namespace files
}   // namespace los

#endif // !LOS_INTERNAL_LOG_LOGGER_H_
