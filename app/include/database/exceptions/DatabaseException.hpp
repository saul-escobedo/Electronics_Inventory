#pragma once

#include <stdexcept>

namespace ecim {
    class DatabaseException : public std::exception {
    public:
        DatabaseException(std::string message) : m_message(message) {}

        const char* what() const noexcept override {
            return m_message.c_str();
        }
    private:
        std::string m_message;
    };
}
