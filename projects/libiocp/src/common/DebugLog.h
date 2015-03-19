#ifndef _DEBUG_LOG_H_
#define _DEBUG_LOG_H_

namespace debug {
    void printfToConsole(const char *tag, const char *fmt, ...);
    void printfToWindow(const char *tag, const char *fmt, ...);
    void printfToConsoleAndWindow(const char *tag, const char *fmt, ...);
}

#endif