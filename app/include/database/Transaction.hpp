#pragma once

namespace ecim {
    /// @brief Encapsulates a transaction session with the database.
    ///
    /// Transactions allow you to group multiple database operations into a
    /// single unit of work. When committing, all changes made within the
    /// transaction are saved permanently. If any error occurs, changes are
    /// rolled back, meaning all operations made since the start of the
    /// transaction are undone. This ensures that the data remains consistent.
    ///
    /// An instance of a transaction is returned from a database
    /// implementation. If the object is destroyed before calling `commit()`,
    /// (i.e. the object goes out of scope) then all the changes will be
    /// reverted.
    ///
    /// Calling `commit()` is nesseary if the changes are meant to be saved.
    class Transaction {
    public:
        virtual void commit() = 0;
        virtual void rollback() = 0;
    };
}
