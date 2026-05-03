#ifndef DATABASE_MANAGER_HPP
#define DATABASE_MANAGER_HPP

#include "database/Database.hpp"
#include "database/SQLiteDatabase.hpp"

#include <QVector>
#include <QString>
#include <memory>

namespace ecim{
    class DatabaseManager {
    public:
        DatabaseManager();
        ~DatabaseManager();

        bool openDatabase();
        void reopenDatabase();
        bool moveDatabase(const QString &newFolder);

        QString getDatabasePath() const;

        // API passthrough functions
        MassQueryResult getAllComponents(
            const MassQueryConfig& queryConfig = ecim::MassQueryConfig());
        MassQueryResult getAllComponentsByType(
            ElectronicComponent::Type type,
            const MassQueryConfig& queryConfig = MassQueryConfig());

    private:
        std::unique_ptr<ecim::SQLiteDatabase> m_db;
        QString m_dbPath;
        bool m_initialized;
    };
}
#endif // DATABASE_MANAGER_HPP
