#pragma once

#include <stdexcept>

#include <cstdio>
#include <stdarg.h>

namespace ecim {
    class DatabaseException : public std::exception {
    public:
        DatabaseException(std::string message) : m_message(message) {}

        DatabaseException(const char* format, ...) {
            char buf[4096];

            va_list args;
            va_start(args, format);
            vsnprintf(buf, sizeof(buf), format, args);
            va_end(args);
            m_message = buf;
        }

        const char* what() const noexcept override {
            return m_message.c_str();
        }
    private:
        std::string m_message;
    };
}
