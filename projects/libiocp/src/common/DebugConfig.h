#ifndef _DEBUG_CONFIG_H_
#define _DEBUG_CONFIG_H_

#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL 1
#endif

#if (!defined DEBUG_LEVEL) || (DEBUG_LEVEL == 0)

#   define LOG_VERBOSE(fmt, ...) do { } while (0)
#   define LOG_DEBUG(fmt, ...) do { } while (0)
#   define LOG_INFO(fmt, ...) do { } while (0)
#   define LOG_WARN(fmt, ...) do { } while (0)
#   define LOG_ERROR(fmt, ...) do { } while (0)
#   define LOG_ASSERT(expression) do { } while (0)

#elif DEBUG_LEVEL == 4

#   include <assert.h>
#   include "DebugLog.h"

#   define LOG_VERBOSE(fmt, ...) do { } while (0)
#   define LOG_DEBUG(fmt, ...) do { } while (0)
#   define LOG_INFO(fmt, ...) do { } while (0)
#   define LOG_WARN(fmt, ...) do { } while (0)
#   define LOG_ERROR(fmt, ...) debug::printfToWindow("ERROR", fmt, ##__VA_ARGS__)
#   define LOG_ASSERT(expression) assert(expression)

#elif DEBUG_LEVEL == 2

#   include <assert.h>
#   include "DebugLog.h"

#   define LOG_VERBOSE(fmt, ...) do { } while (0)
#   define LOG_DEBUG(fmt, ...) do { } while (0)
#   define LOG_INFO(fmt, ...) do { } while (0)
#   define LOG_WARN(fmt, ...) debug::printfToWindow("WARN", fmt, ##__VA_ARGS__)
#   define LOG_ERROR(fmt, ...) debug::printfToConsoleAndWindow("ERROR", fmt, ##__VA_ARGS__)
#   define LOG_ASSERT(expression) assert(expression)

#elif DEBUG_LEVEL == 3

#   include <assert.h>
#   include "DebugLog.h"

#   define LOG_VERBOSE(fmt, ...) do { } while (0)
#   define LOG_DEBUG(fmt, ...) do { } while (0)
#   define LOG_INFO(fmt, ...) debug::printfToWindow("INFO", fmt, ##__VA_ARGS__)
#   define LOG_WARN(fmt, ...) debug::printfToConsoleAndWindow("WARN", fmt, ##__VA_ARGS__)
#   define LOG_ERROR(fmt, ...) debug::printfToConsoleAndWindow("ERROR", fmt, ##__VA_ARGS__)
#   define LOG_ASSERT(expression) assert(expression)

#elif DEBUG_LEVEL == 4

#   include <assert.h>
#   include "DebugLog.h"

#   define LOG_VERBOSE(fmt, ...) do { } while (0)
#   define LOG_DEBUG(fmt, ...) debug::printfToConsoleAndWindow("DEBUG", fmt, ##__VA_ARGS__)
#   define LOG_INFO(fmt, ...) debug::printfToWindow("INFO", fmt, ##__VA_ARGS__)
#   define LOG_WARN(fmt, ...) debug::printfToConsoleAndWindow("WARN", fmt, ##__VA_ARGS__)
#   define LOG_ERROR(fmt, ...) debug::printfToConsoleAndWindow("ERROR", fmt, ##__VA_ARGS__)
#   define LOG_ASSERT(expression) assert(expression)

#else

#   include <assert.h>
#   include "DebugLog.h"

#   define LOG_VERBOSE(fmt, ...) debug::printfToConsole("VERBOSE", fmt, ##__VA_ARGS__)
#   define LOG_DEBUG(fmt, ...) debug::printfToConsoleAndWindow("DEBUG", fmt, ##__VA_ARGS__)
#   define LOG_INFO(fmt, ...) debug::printfToWindow("INFO", fmt, ##__VA_ARGS__)
#   define LOG_WARN(fmt, ...) debug::printfToConsoleAndWindow("WARN", fmt, ##__VA_ARGS__)
#   define LOG_ERROR(fmt, ...) debug::printfToConsoleAndWindow("ERROR", fmt, ##__VA_ARGS__)
#   define LOG_ASSERT(expression) assert(expression)

#endif

#endif
