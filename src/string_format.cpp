#include "string_format.h"
#include <memory>
#include <stdarg.h>

// http://stackoverflow.com/questions/23157530/vsnprintf-on-mac-gives-exc-bad-access
std::string string_format(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    size_t size = std::vsnprintf(nullptr, 0, fmt, args) + 1; // Extra space for '\0'
    va_end(args);
    va_start(args, fmt);
    std::unique_ptr<char[]> buf( new char[ size ] ); 
    std::vsnprintf( buf.get(), size, fmt, args);
    va_end(args);
    return std::string( buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}
