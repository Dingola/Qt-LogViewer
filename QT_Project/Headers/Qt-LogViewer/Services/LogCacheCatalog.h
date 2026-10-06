#pragma once

#include <QObject>
#include <QString>
#include <QUuid>
#include <QVector>
#include <optional>

#include "Qt-LogViewer/Services/LogCacheIdentity.h"

/**
 * @file LogCacheCatalog.h
 * @brief Declares the versioned metadata catalog for persistent per-file
 * caches.
 */

/**
 * @enum LogCacheGenerationState
 * @brief Describes whether a persistent cache generation may be exposed to a
 * view.
 */
enum class LogCacheGenerationState
{
    /** @brief The generation is being populated and must remain hidden from
     * views. */
    Building,

    /** @brief The generation was finalized successfully and can be reused. */
    Complete,

    /** @brief Population was cancelled, interrupted or failed. */
    Failed
};

/**
 * @struct LogCacheGeneration
 * @brief Describes one catalogued revision of a source file and parser
 * configuration.
 */
struct LogCacheGeneration {
        /** @brief Catalog primary key of this generation. */
        qint64 id{-1};

        /** @brief Catalog primary key of the canonical source path. */
        qint64 source_id{-1};

        /** @brief Exact source and parser identity represented by the generation. */
        LogCacheIdentity identity;

        /** @brief Absolute path of the disposable per-file SQLite database. */
        QString database_path;

        /** @brief Current lifecycle state controlling whether views may use the
         * generation. */
        LogCacheGenerationState state{LogCacheGenerationState::Building};

        /** @brief Number of source bytes covered by the finalized index. */
        qint64 indexed_bytes{0};

        /** @brief Number of indexed log records. */
        qint64 entry_count{0};

        /** @brief Measured on-disk storage consumption in bytes. */
        qint64 storage_bytes{0};

        /** @brief Last access time as milliseconds since the UTC epoch. */
        qint64 last_access_utc_ms{0};

        /** @brief Diagnostic text retained for failed generations. */
        QString error_message;
};

/**
 * @class LogCacheCatalog
 * @brief Stores small cache metadata separately from disposable per-file cache
 * databases.
 */
class LogCacheCatalog final: public QObject
{
        Q_OBJECT

    public:
        /** @brief SQLite schema version understood by this catalog implementation. */
        static constexpr int SchemaVersion = 2;

        /**
         * @brief Opens the default application cache catalog.
         * @param parent Optional QObject parent.
         */
        explicit LogCacheCatalog(QObject* parent = nullptr);

        /**
         * @brief Opens a cache catalog below an explicit root directory.
         * @param cache_root Directory containing the catalog and per-file databases.
         * @param parent Optional QObject parent.
         */
        explicit LogCacheCatalog(QString cache_root, QObject* parent = nullptr);

        /** @brief Closes the private SQLite connection. */
        ~LogCacheCatalog() override;

        /**
         * @brief Returns whether the catalog schema is available.
         * @return True when the directory, SQLite connection and schema were
         * initialized.
         */
        [[nodiscard]] auto is_available() const -> bool;

        /**
         * @brief Returns the cache root directory.
         * @return Absolute directory containing the catalog and the `files`
         * subdirectory.
         */
        [[nodiscard]] auto get_cache_root() const -> QString;

        /**
         * @brief Returns the central catalog database path.
         * @return Absolute path of `catalog.sqlite`.
         */
        [[nodiscard]] auto get_database_path() const -> QString;

        /**
         * @brief Returns the current SQLite user schema version.
         * @return `PRAGMA user_version`, or zero when the catalog is unavailable.
         */
        [[nodiscard]] auto get_schema_version() const -> int;

        /**
         * @brief Registers or resumes the generation for an exact cache identity.
         * @param identity Valid source and parser identity.
         * @return Generation metadata, or std::nullopt when registration fails.
         */
        auto begin_generation(const LogCacheIdentity& identity)
            -> std::optional<LogCacheGeneration>;

        /**
         * @brief Returns the complete generation matching an exact identity.
         * @param identity Source and parser identity to find.
         * @return Complete generation metadata, or std::nullopt when no reusable
         * cache exists.
         */
        [[nodiscard]] auto find_complete_generation(const LogCacheIdentity& identity) const
            -> std::optional<LogCacheGeneration>;

        /**
         * @brief Finds the largest complete generation that is an unchanged source prefix.
         * @param identity Current source and parser identity.
         * @return Largest reusable prefix generation, or std::nullopt when none is safe.
         */
        [[nodiscard]] auto find_complete_prefix(const LogCacheIdentity& identity) const
            -> std::optional<LogCacheGeneration>;

        /**
         * @brief Returns one generation by its catalog identifier.
         * @param generation_id Catalog generation primary key.
         * @return Generation metadata, or std::nullopt when unavailable or not found.
         */
        [[nodiscard]] auto get_generation(qint64 generation_id) const
            -> std::optional<LogCacheGeneration>;

        /**
         * @brief Atomically marks a building generation complete and records measured
         * sizes.
         * @param generation_id Catalog generation primary key.
         * @param indexed_bytes Number of source bytes represented by the cache.
         * @param entry_count Number of indexed log records.
         * @param storage_bytes Total measured cache storage in bytes.
         * @return True only when exactly one building generation was finalized.
         */
        auto mark_complete(qint64 generation_id, qint64 indexed_bytes, qint64 entry_count,
                           qint64 storage_bytes) -> bool;

        /**
         * @brief Marks a building generation failed without exposing it to views.
         * @param generation_id Catalog generation primary key.
         * @param error_message Diagnostic reason for the failure.
         * @return True only when exactly one building generation was updated.
         */
        auto mark_failed(qint64 generation_id, const QString& error_message) -> bool;

        /**
         * @brief Marks an incomplete generation failed and removes its disposable database.
         * @param generation_id Catalog generation primary key.
         * @param error_message Diagnostic reason for discarding the generation.
         * @return True when metadata was updated and all existing cache files were removed.
         */
        auto discard_generation(qint64 generation_id, const QString& error_message) -> bool;

        /**
         * @brief Refreshes the LRU timestamp of one generation.
         * @param generation_id Catalog generation primary key.
         * @return True when the generation exists and was touched.
         */
        auto touch_generation(qint64 generation_id) -> bool;

        /**
         * @brief Replaces a view's ordered mapping with complete cache generations.
         * @param view_id Stable view identifier.
         * @param generation_ids Complete generations in view order.
         * @return True when the complete ordered mapping was committed atomically.
         */
        auto bind_view(const QUuid& view_id, const QVector<qint64>& generation_ids) -> bool;

        /**
         * @brief Returns complete generation identifiers bound to a view in display
         * order.
         * @param view_id Stable view identifier.
         * @return Ordered catalog generation primary keys, or an empty vector if none
         * exist.
         */
        [[nodiscard]] auto get_view_generations(const QUuid& view_id) const -> QVector<qint64>;

        /**
         * @brief Removes a view mapping without deleting reusable cache generations.
         * @param view_id Stable view identifier.
         * @return True when the delete statement completed successfully.
         */
        auto remove_view(const QUuid& view_id) -> bool;

        /**
         * @brief Returns the standard application cache root.
         * @return Absolute `history/cache-v2` path below
         * QStandardPaths::AppConfigLocation.
         */
        [[nodiscard]] static auto get_default_cache_root() -> QString;

    private:
        /**
         * @brief Opens, configures and migrates the private SQLite connection.
         * @return True when the catalog connection and current schema are usable.
         */
        auto initialize_database() -> bool;

        /**
         * @brief Replaces an incompatible catalog schema and removes its stale cache files.
         * @return True when the replacement schema was committed successfully.
         */
        auto rebuild_schema() -> bool;

        /**
         * @brief Creates all catalog tables and indexes if they do not exist.
         * @return True when every schema statement completed successfully.
         */
        auto create_schema() -> bool;

        /** @brief Removes disposable databases belonging to an incompatible catalog
         * schema. */
        auto remove_stale_cache_files() -> void;

        /**
         * @brief Loads one generation and reconstructs its exact cache identity.
         * @param generation_id Catalog generation primary key.
         * @return Generation metadata, or std::nullopt when unavailable or not found.
         */
        [[nodiscard]] auto load_generation(qint64 generation_id) const
            -> std::optional<LogCacheGeneration>;

        /**
         * @brief Resolves a cache filename below the managed `files` directory.
         * @param cache_key Cache filename or key-derived filename to resolve.
         * @return Absolute path below the catalog's managed `files` directory.
         */
        [[nodiscard]] auto get_cache_database_path(const QString& cache_key) const -> QString;

        /** @brief Unique Qt SQL connection name owned by this instance. */
        QString m_connection_name;
        /** @brief Absolute directory containing catalog and cache files. */
        QString m_cache_root;
        /** @brief Absolute path of the catalog SQLite database. */
        QString m_database_path;
        /** @brief Whether initialization completed and public operations may execute.
         */
        bool m_is_available{false};
};
