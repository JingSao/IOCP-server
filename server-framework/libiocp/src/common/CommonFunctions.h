#ifndef _COMMON_FUNCTIONS_H_
#define _COMMON_FUNCTIONS_H_

namespace Common {
    void consoleprintf(const char *tag, const char *fmt, ...);
    void debugprintf(const char *tag, const char *fmt, ...);
    void allprintf(const char *tag, const char *fmt, ...);
}

#endif