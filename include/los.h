#ifndef LOS_INCLUDE_LOS_H_
#define LOS_INCLUDE_LOS_H_

#include <stdint.h>
#include <stddef.h>
#include <memory>

#if defined(_WIN32)
#if defined(LOS_EXPORT)
#define LOS_API __declspec(dllexport)
#else
#define LOS_API __declspec(dllimport)
#endif
#else
#define LOS_API
#endif

#endif // !LOS_INCLUDE_LOS_H_
