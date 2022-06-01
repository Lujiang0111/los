#include "log/logger.h"

#include <time.h>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#include <sys/syscall.h>
#endif

#include "log/log_thread.h"
#include "fmt/color.h"

namespace los {
namespace logs {

static constexpr struct LogLevelMaps
{
    fmt::terminal_color fg_color;
    const char *level_msg;
}kLogLevelMaps[] =
{
    {fmt::terminal_color::bright_green, "DEBUG"},
    {fmt::terminal_color::bright_blue, "INFO "},
    {fmt::terminal_color::bright_yellow, "WARN "},
    {fmt::terminal_color::bright_red, "ERROR"},
};

// Return current thread id as size_t
// It exists because the std::this_thread::get_id() is much slower(especially
// under VS 2013)
static inline size_t GetThreadId_()
{
#if defined(_WIN32)
    return static_cast<size_t>(::GetCurrentThreadId());
#elif defined(__linux__)
#    if defined(__ANDROID__) && defined(__ANDROID_API__) && (__ANDROID_API__ < 21)
#        define SYS_gettid __NR_gettid
#    endif
    return static_cast<size_t>(::syscall(SYS_gettid));
#elif defined(_AIX) || defined(__DragonFly__) || defined(__FreeBSD__)
    return static_cast<size_t>(::pthread_getthreadid_np());
#elif defined(__NetBSD__)
    return static_cast<size_t>(::_lwp_self());
#elif defined(__OpenBSD__)
    return static_cast<size_t>(::getthrid());
#elif defined(__sun)
    return static_cast<size_t>(::thr_self());
#elif __APPLE__
    uint64_t tid;
    pthread_threadid_np(nullptr, &tid);
    return static_cast<size_t>(tid);
#else // Default to standard C++11 (other Unix)
    return static_cast<size_t>(std::hash<std::thread::id>()(std::this_thread::get_id()));
#endif
}

static inline size_t GetThreadId()
{
    static thread_local const size_t tid = GetThreadId_();
    return tid;
}

Logger::Logger(const char *path, size_t max_size)
{
    path_ = ((path) && (0 != *path)) ? path : "Log";
    AdjustFilePath(path_);

    max_size_ = max_size;
    hour_count_ = 0;
    file_ = nullptr;
}

Logger::~Logger()
{
    if (file_)
    {
        fclose(file_);
        file_ = nullptr;
    }
}

void Logger::Log(Levels level, bool print_screen, const char *name, int line, const char *format, ...)
{
    std::vector<char> content_buf(256);
    va_list vl;
    va_start(vl, format);
    int content_len = VsprintfForVector(content_buf, format, vl);
    va_end(vl);

    LogContent(level, print_screen, name, line, &content_buf[0], static_cast<size_t>(content_len));
}

void Logger::LogContent(Levels level, bool print_screen, const char *name, int line, const char *content, size_t content_length)
{
    LogMsg msg;
    msg.logger = shared_from_this();
    msg.type = kLog;
    msg.time = std::chrono::system_clock::now();
    msg.thread_id = GetThreadId();
    msg.level = level;
    msg.print_screen = print_screen;
    msg.name = (name) ? name : "";
    msg.line = line;

    if ((content) && (content_length > 0))
    {
        msg.content.assign(content, content_length);
    }

    LogThread::GetInstance().Enqueue(std::move(msg));
}

void Logger::DoLog(size_t id, const LogMsg &msg)
{
    time_t time_tt = std::chrono::system_clock::to_time_t(msg.time);
    std::tm time_tm;
#if defined(_WIN32)
    ::localtime_s(&time_tm, &time_tt);
#else
    ::localtime_r(&time_tt, &time_tm);
#endif

    int64_t hour_count = std::chrono::duration_cast<std::chrono::hours>(msg.time.time_since_epoch()).count();
    if (hour_count != hour_count_)
    {
        // close exist file
        if (file_)
        {
            fclose(file_);
            file_ = nullptr;
        }

        // do cleaning
        if (max_size_ > 0)
        {
            auto file_info = files::GetFileInfo(path_.c_str(), files::kByName);
            if ((file_info) && (file_info->GetTotalSize() > max_size_))
            {
                size_t clear_size = file_info->GetTotalSize() - max_size_ / 10 * 8;
                DeleteLog(file_info.get(), clear_size);
            }
        }

        // create new file
        auto name = fmt::format("{}{}{:04d}-{:02d}-{:02d}{}{:04d}-{:02d}-{:02d}-{:02d}.log",
            path_, kDirSep,
            time_tm.tm_year + 1900, time_tm.tm_mon + 1, time_tm.tm_mday, kDirSep,
            time_tm.tm_year + 1900, time_tm.tm_mon + 1, time_tm.tm_mday, time_tm.tm_hour);
        files::CreateDir(name.c_str(), true);
        file_ = fopen(name.c_str(), "a");

        hour_count_ = hour_count;
    }

    if (msg.print_screen)
    {
        if (msg.name.length() > 0)
        {
            if (msg.line > 0)
            {
                fmt::print(fg(kLogLevelMaps[msg.level].fg_color),
                    "[{}] {} {}:{}, T:{}, {}\n",
                    id, kLogLevelMaps[msg.level].level_msg, msg.name, msg.line, msg.thread_id, msg.content);
            }
            else
            {
                fmt::print(fg(kLogLevelMaps[msg.level].fg_color),
                    "[{}] {} {}, T:{}, {}\n",
                    id, kLogLevelMaps[msg.level].level_msg, msg.name, msg.thread_id, msg.content);
            }
        }
        else
        {
            fmt::print(fg(kLogLevelMaps[msg.level].fg_color),
                "[{}] {}, T:{}, {}\n",
                id, kLogLevelMaps[msg.level].level_msg, msg.thread_id, msg.content);
        }
    }

    int msec = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(msg.time.time_since_epoch()).count() % 1000);
    if (file_)
    {
        std::string content = (msg.name.length() > 0)
            ? fmt::format("[{}] {:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d} {:03d}: {} {}:{}, T:{}, {}\n",
                id,
                time_tm.tm_year + 1900, time_tm.tm_mon + 1, time_tm.tm_mday,
                time_tm.tm_hour, time_tm.tm_min, time_tm.tm_sec, msec,
                kLogLevelMaps[msg.level].level_msg, msg.name, msg.line, msg.thread_id, msg.content)
            : fmt::format("[{}] {:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d} {:03d}: {}, T:{}, {}\n",
                id,
                time_tm.tm_year + 1900, time_tm.tm_mon + 1, time_tm.tm_mday,
                time_tm.tm_hour, time_tm.tm_min, time_tm.tm_sec, msec,
                kLogLevelMaps[msg.level].level_msg, msg.thread_id, msg.content);
        fwrite(content.c_str(), 1, content.length(), file_);
        fflush(file_);
    }
}

void Logger::DeleteLog(const files::FileInfoInterface *file_info, size_t &del_size)
{
    if (0 == del_size)
    {
        return;
    }

    if ((file_info->GetTotalSize() < del_size) || (files::kRegular == file_info->GetMode()))
    {
        files::RemoveFile(file_info->GetFullName());
        del_size = (del_size > file_info->GetTotalSize()) ? (del_size - file_info->GetTotalSize()) : 0;
    }
    else if (files::kDirectory == file_info->GetMode())
    {
        for (size_t i = 0; i < file_info->GetChildsSize(); ++i)
        {
            DeleteLog(file_info->GetChild(i), del_size);
        }
    }
}

}   // namespace logs
}   // namespace los
