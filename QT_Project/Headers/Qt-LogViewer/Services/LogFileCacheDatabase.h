#pragma once

#include <QObject>
#include <QString>

#include "Qt-LogViewer/Services/LogCacheIdentity.h"

/**
 * @file LogFileCacheDatabase.h
 * @brief Declares one disposable, normalized per-file cache database.
 */

/**
 * @class LogFileCacheDatabase
 * @brief Owns the schema boundary for entries belonging to exactly one file
 * generation.
 */
class LogFileCacheDatabase final: public QObject
{
        Q_OBJECT

    public:
        /** @brief SQLite schema version understood by this per-file cache
         * implementation. */
        static constexpr int SchemaVersion = 2;

        /**
         * @brief Opens or creates a per-file cache database for an exact identity.
         * @param database_path Destination SQLite path.
         * @param identity File and parser identity represented by this database.
         * @param parent Optional QObject parent.
         */
        LogFileCacheDatabase(QString database_path, LogCacheIdentity identity,
                             QObject* parent = nullptr);

        /** @brief Closes the private SQLite connection. */
        ~LogFileCacheDatabase() override;

        /**
         * @brief Returns whether the database schema and stored identity are usable.
         * @return True when initialization succeeded and the stored identity matches
         * exactly.
         */
        [[nodiscard]] auto is_available() const -> bool;

        /**
         * @brief Returns the per-file SQLite database path.
         * @return Absolute database path supplied to the constructor.
         */
        [[nodiscard]] auto get_database_path() const -> QString;

        /**
         * @brief Returns the current SQLite user schema version.
         * @return `PRAGMA user_version`, or zero when the database is unavailable.
         */
        [[nodiscard]] auto get_schema_version() const -> int;

        /**
         * @brief Returns the exact identity expected from this database.
         * @return Immutable source and parser identity owned by this instance.
         */
        [[nodiscard]] auto get_identity() const -> const LogCacheIdentity&;

    private:
        /**
         * @brief Opens, configures and validates the private SQLite connection.
         * @return True when the schema exists and its stored identity matches the expected one.
         */
        auto initialize_database() -> bool;

        /**
         * @brief Replaces an incompatible schema and persists the expected identity.
         * @return True when the empty replacement schema and identity were committed.
         */
        auto rebuild_schema() -> bool;

        /**
         * @brief Creates normalized entry tables, lookup indexes and contentless FTS metadata.
         * @return True when every schema statement completed successfully.
         */
        auto create_schema() -> bool;

        /**
         * @brief Stores the exact source and parser identity in the singleton metadata row.
         * @return True when the identity row was written successfully.
         */
        auto store_identity() -> bool;

        /**
         * @brief Compares every stored identity component with the expected identity.
         * @return True only when every persisted component matches exactly.
         */
        [[nodiscard]] auto identity_matches() const -> bool;

        /** @brief Unique Qt SQL connection name owned by this instance. */
        QString m_connection_name;
        /** @brief Absolute path of the disposable per-file cache database. */
        QString m_database_path;
        /** @brief Source and parser identity that the database must represent. */
        LogCacheIdentity m_identity;
        /** @brief Whether schema initialization and identity validation succeeded. */
        bool m_is_available{false};
};
