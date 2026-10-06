#pragma once

#include <QHash>
#include <QObject>
#include <QString>
#include <QVector>

#include "Qt-LogViewer/Models/LogEntry.h"
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

        /**
         * @brief Copies a finalized generation and retargets its identity for an appended source.
         * @param source_database_path Finalized prefix database to copy.
         * @param destination_database_path Building generation database to create.
         * @param destination_identity Identity of the current appended source snapshot.
         * @return True when the database was copied and its identity updated atomically.
         */
        static auto clone_generation(const QString& source_database_path,
                                     const QString& destination_database_path,
                                     const LogCacheIdentity& destination_identity) -> bool;

        /**
         * @brief Removes all indexed entries before a generation is rebuilt.
         * @return True when the old contents were cleared and a generation-wide transaction
         * started.
         */
        auto reset_entries() -> bool;

        /**
         * @brief Starts a generation-wide transaction without clearing cloned entries.
         * @return True when the database is ready to append suffix entries.
         */
        auto begin_append() -> bool;

        /**
         * @brief Appends one parsed batch to the normalized index and contentless FTS table.
         * @param entries Parsed entries carrying exact source byte ranges.
         * @return True when the complete batch was staged in the generation-wide transaction.
         */
        auto append_entries(const QVector<LogEntry>& entries) -> bool;

        /**
         * @brief Flushes the completed index and truncates its WAL file.
         * @return True when optimization and the final checkpoint completed successfully.
         */
        auto finalize_writes() -> bool;

        /**
         * @brief Counts indexed records.
         * @return Number of rows in `log_entries`, or -1 when the query fails.
         */
        [[nodiscard]] auto get_entry_count() const -> qint64;

        /**
         * @brief Measures the database and its SQLite sidecar files.
         * @return Combined size of the database, WAL and shared-memory files in bytes.
         */
        [[nodiscard]] auto get_storage_bytes() const -> qint64;

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

        /**
         * @brief Returns or creates a normalized log-level identifier.
         * @param value Original log-level text.
         * @return Positive row identifier, or -1 when lookup or insertion fails.
         */
        auto get_or_create_level_id(const QString& value) -> qint64;

        /**
         * @brief Returns or creates an application identifier.
         * @param value Original application name.
         * @return Positive row identifier, or -1 when lookup or insertion fails.
         */
        auto get_or_create_application_id(const QString& value) -> qint64;

        /** @brief Unique Qt SQL connection name owned by this instance. */
        QString m_connection_name;
        /** @brief Absolute path of the disposable per-file cache database. */
        QString m_database_path;
        /** @brief Source and parser identity that the database must represent. */
        LogCacheIdentity m_identity;
        /** @brief Whether schema initialization and identity validation succeeded. */
        bool m_is_available{false};
        /** @brief Whether a generation-wide atomic write transaction is active. */
        bool m_write_transaction_active{false};
        /** @brief In-memory dimension lookup avoiding repeated SELECT statements per batch. */
        QHash<QString, qint64> m_level_ids;
        /** @brief In-memory application lookup avoiding repeated SELECT statements per batch. */
        QHash<QString, qint64> m_application_ids;
};
