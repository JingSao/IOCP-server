#ifndef _EXCEPTIONS_H_
#define _EXCEPTIONS_H_

#include <string.h>
#include <stdexcept>

namespace iocp {

    template <size_t _N> class Exception final : public std::exception
    {
    public:
        Exception(const char (&str)[_N]) { strcpy(_what, str); }
        virtual const char *what() const throw() override { return _what; }
    private:
        char _what[_N];
    };
}

#define MAKE_EXCEPTION(str) iocp::Exception<sizeof(str)>(str)

#endif
