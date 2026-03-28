#include "DatabaseException.hpp"

#include <cstdio>

namespace ecim {
    class FailedInitializationException : public DatabaseException {
    private:
        std::string _formatMessage(const std::string& backendName, const std::string& log) {
            char message[512];

            snprintf(
                message,
                sizeof(message),
                "Initialization of a %s database failed. %s",
                backendName.c_str(),
                !log.empty() ? "Error log:\n\n" : ""
            );

            return message + log;
        }
    public:
        FailedInitializationException(const std::string& backendName, const std::string& log = "")
        : DatabaseException(_formatMessage(backendName, log)) {}
    };
}
