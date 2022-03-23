#ifndef LOS_INTERNAL_CORES_H_
#define LOS_INTERNAL_CORES_H_

#include <stdarg.h>
#include <string>
#include <vector>
#include <algorithm>

constexpr char kDirSeps[] = { '\\', '/' };

#if defined(_WIN32)
constexpr char kDirSep = kDirSeps[0];
#else
constexpr char kDirSep = kDirSeps[1];
#endif

// Replace directory separators to current platform
inline void AdjustFilePath(std::string &name)
{
    for (auto &&sep : kDirSeps)
    {
        if (sep != kDirSep)
        {
            std::replace(name.begin(), name.end(), sep, kDirSep);
        }
    }

    // Delete directory separators at the end
    while (kDirSep == name.back())
    {
        name.pop_back();
    }
}

/***************************************************************************//**
使用方法：
std::vector<char> buf;
va_list vl;
va_start(vl, fmt);
int len = VsprintfForVector(buf, fmt, vl);
va_end(vl);
 ******************************************************************************/
inline int VsprintfForVector(std::vector<char> &vec, const char *fmt, va_list &vl)
{
    if (vec.empty())
    {
        vec.resize(256);
    }

    int ret = 0;
    while (true)
    {
        va_list vp;
        va_copy(vp, vl);
        ret = vsnprintf(&vec[0], vec.size(), fmt, vp);
        va_end(vp);

        if (ret >= static_cast<int>(vec.size()))
        {
            vec.resize(ret + 1);
        }
        else if (ret > 0)
        {
            break;
        }
    }

    return ret;
}

#endif // !LOS_INTERNAL_CORES_H_
