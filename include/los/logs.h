﻿#ifndef LOS_INCLUDE_LOS_LOGS_H_
#define LOS_INCLUDE_LOS_LOGS_H_

#include <stdarg.h>

#include "los.h"

namespace los {
namespace logs {

enum Levels
{
    kDebug = 0,
    kInfo,
    kWarn,
    kError,
    kLevelCnt,
};

class LOS_API LoggerInterface
{
public:
    virtual ~LoggerInterface() = default;

    virtual void Log(Levels level, bool print_screen, const char *name, int line, const char *format, ...) = 0;
    virtual void LogContent(Levels level, bool print_screen, const char *name, int line, const char *content, size_t content_length) = 0;
};

/***************************************************************************//**
 * 创建一个新的日志句柄
 * @param   path        [in]    日志存放目录
 * @param   max_size    [in]    日志的总大小，为0则不限制
 * @return  日志句柄
 ******************************************************************************/
LOS_API std::shared_ptr<LoggerInterface> CreateLogger(const char *path, size_t max_size);

/***************************************************************************//**
 * 获取默认句柄，日志默认为"Log"，大小不限制
 * @return  默认日志句柄
 ******************************************************************************/
LOS_API LoggerInterface *DefaultLogger();

/***************************************************************************//**
 * printf封装，异步接口，使打印出来的内容与日志内容不发生错位
 ******************************************************************************/
LOS_API void Printf(const char *format, ...);

/***************************************************************************//**
 * printf封装，带换行，异步接口，使打印出来的内容与日志内容不发生错位
 ******************************************************************************/
LOS_API void Printfln(const char *format, ...);

}   // namespace logs
}   // namespace los

#endif // !LOS_INCLUDE_LOS_LOGS_H_
