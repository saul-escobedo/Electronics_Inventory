#include "DatabaseException.hpp"

#include <cstdio>

namespace ecim {
    class UninitializedDatabaseException : public DatabaseException {
    private:
        std::string _formatMessage(const std::string& backendName) {
            char message[512];

            snprintf(
                message,
                sizeof(message),
                "An accessor function in an uninitialized %s database was used.",
                backendName.c_str()
            );

            return message;
        }
    public:
        UninitializedDatabaseException(const std::string& backendName)
        : DatabaseException(_formatMessage(backendName)) {}
    };
}
