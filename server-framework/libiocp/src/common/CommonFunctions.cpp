#include <windows.h>
#include <stdio.h>
#include "CommonFunctions.h"

namespace Common {
    void consoleprintf(const char *tag, const char *fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        printf("[%s]", tag);
        vprintf(fmt, args);
        va_end(args);
    }

    void debugprintf(const char *tag, char *fmt, ...)
    {
        char buf[1024];
        int n = _snprintf(buf, 1024, "[%s]", tag);
        va_list args;
        va_start(args, fmt);
        int ret = _vsnprintf_s(buf + n, 1024 - n, 1024 - n, fmt, args);
        OutputDebugStringA(buf);
        OutputDebugStringA("\n");
        va_end(args);
    }

    void allprintf(const char *tag, const char *fmt, ...)
    {
        char buf[1024];
        int n = _snprintf(buf, 1024, "[%s]", tag);
        va_list args;
        va_start(args, fmt);
        int ret = _vsnprintf_s(buf + n, 1024 - n, 1024 - n, fmt, args);
        OutputDebugStringA(buf);
        OutputDebugStringA("\n");
        puts(buf);
        va_end(args);
    }
}
