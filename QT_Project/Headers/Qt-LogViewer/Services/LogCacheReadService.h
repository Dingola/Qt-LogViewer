#pragma once

#include <QHash>
#include <QMap>
#include <QSet>
#include <QString>
#include <QUuid>
#include <QVector>

#include "Qt-LogViewer/Models/LogEntry.h"
#include "Qt-LogViewer/Models/LogQuery.h"

class LogCacheCatalog;
class ViewRegistry;

/**
 * @file LogCacheReadService.h
 * @brief Declares read access across complete per-file cache generations.
 */

/**
 * @class LogCacheReadService
 * @brief Queries complete cache generations and materializes visible source records on demand.
 */
class LogCacheReadService final
{
    public:
        /**
         * @brief Constructs the cache reader.
         * @param catalog Catalog resolving view-to-generation mappings.
         * @param views Registry providing file order and parser profiles.
         */
        explicit LogCacheReadService(LogCacheCatalog* catalog, ViewRegistry* views);

        /**
         * @brief Checks whether a view has a complete, readable and current cache mapping.
         * @param view_id View to validate.
         * @return True when every registered file has an exact complete generation.
         */
        [[nodiscard]] auto can_query(const QUuid& view_id) const -> bool;

        /**
         * @brief Counts cached entries matching a view query.
         * @param query Filter and search state.
         * @return Number of matching entries.
         */
        [[nodiscard]] auto count_entries(const LogQuery& query) const -> qsizetype;

        /**
         * @brief Loads and materializes one globally sorted page from the source files.
         * @param query Filter, search and sorting state.
         * @param offset Zero-based result offset.
         * @param limit Maximum number of entries to return.
         * @return Matching entries in requested order.
         */
        [[nodiscard]] auto load_entries_page(const LogQuery& query, qsizetype offset,
                                             qsizetype limit) const -> QVector<LogEntry>;

        /**
         * @brief Counts matching cached entries grouped by normalized level.
         * @param query Query whose level selection is ignored for faceting.
         * @return Uppercase level names and counts.
         */
        [[nodiscard]] auto get_log_level_counts(const LogQuery& query) const
            -> QMap<QString, qsizetype>;

        /**
         * @brief Returns distinct cached values for a standard filter field.
         * @param view_id View to inspect.
         * @param field_id Stable field identifier.
         * @return Distinct non-empty values.
         */
        [[nodiscard]] auto get_distinct_values(const QUuid& view_id,
                                               const QString& field_id) const -> QSet<QString>;

        /**
         * @brief Adds newly tailed entries above an immutable complete cache generation.
         * @param view_id View receiving live entries.
         * @param entries Parsed live entries to expose to subsequent queries.
         */
        auto append_live_entries(const QUuid& view_id, const QVector<LogEntry>& entries) -> void;

        /**
         * @brief Removes all in-memory live entries belonging to a view.
         * @param view_id View whose live overlay is discarded.
         */
        auto remove_view(const QUuid& view_id) -> void;

        /**
         * @brief Removes in-memory live entries belonging to one source file.
         * @param view_id View owning the source.
         * @param file_path Absolute source path to remove.
         */
        auto remove_file(const QUuid& view_id, const QString& file_path) -> void;

    private:
        /** @brief Catalog that resolves persistent generations bound to a view. */
        LogCacheCatalog* m_catalog{nullptr};
        /** @brief Registry supplying ordered source paths and their parser profiles. */
        ViewRegistry* m_views{nullptr};
        /** @brief Newly tailed entries layered above immutable cache generations. */
        QHash<QUuid, QVector<LogEntry>> m_live_entries;
};
