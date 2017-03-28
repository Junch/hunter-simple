#include "string_format.h"
#include <memory>
#include <stdarg.h>

std::string string_format(const char* fmt_str, ...) {
    va_list args;
    va_start(args, fmt_str);
    size_t size = std::vsnprintf(nullptr, 0, fmt_str, args) + 1; // Extra space for '\0'
    std::unique_ptr<char[]> buf( new char[ size ] ); 
    std::vsnprintf( buf.get(), size, fmt_str, args);
    va_end(args);
    return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
}
