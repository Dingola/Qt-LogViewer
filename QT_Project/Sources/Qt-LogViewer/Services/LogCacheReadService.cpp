/**
 * @file LogCacheReadService.cpp
 * @brief Implements merged reads from complete per-file cache generations.
 */

#include "Qt-LogViewer/Services/LogCacheReadService.h"

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QStringList>
#include <QTimeZone>
#include <QUuid>
#include <algorithm>
#include <limits>
#include <optional>

#include "Qt-LogViewer/Controllers/LogViewContext.h"
#include "Qt-LogViewer/Controllers/ViewRegistry.h"
#include "Qt-LogViewer/Models/LogFieldDefinition.h"
#include "Qt-LogViewer/Models/LogFileInfo.h"
#include "Qt-LogViewer/Services/LogCacheCatalog.h"
#include "Qt-LogViewer/Services/LogParser.h"
#include "Qt-LogViewer/Services/LogParsingProfile.h"

namespace
{
/** @brief Fully resolved source backing one ordered part of a cached view. */
struct CacheSource {
        /** @brief Complete persistent generation containing indexed row metadata. */
        LogCacheGeneration generation;
        /** @brief Parser used to materialize visible source records. */
        std::optional<LogParsingProfile> profile;
        /** @brief Absolute path of the immutable source represented by the generation. */
        QString file_path;
        /** @brief Stable source position inside a multi-file view. */
        qsizetype position{0};
};

/** @brief Lightweight cached metadata with optional materialized source content. */
struct CacheRow {
        /** @brief Source generation owning the row. */
        CacheSource source;
        /** @brief Persistent row identifier inside the source generation. */
        qint64 entry_id{-1};
        /** @brief One-based record position in the source file. */
        qsizetype source_line{-1};
        /** @brief Zero-based byte offset of the original source record. */
        qint64 byte_offset{-1};
        /** @brief Byte length of the original source record without its terminator. */
        qint64 byte_length{-1};
        /** @brief Parsed UTC timestamp used by metadata-only sorting. */
        QDateTime timestamp;
        /** @brief Original log-level value stored in the normalized dimension table. */
        QString level;
        /** @brief Original application value stored in the normalized dimension table. */
        QString app_name;
        /** @brief Parsed entry populated only when its complete contents are required. */
        std::optional<LogEntry> materialized_entry;
};

/**
 * @brief Escapes one FTS5 prefix-search token.
 * @param token User-entered token.
 * @return Quoted FTS token followed by a prefix wildcard.
 */
[[nodiscard]] auto escape_fts_token(QString token) -> QString
{
    token.replace(QLatin1Char('"'), QStringLiteral("\"\""));
    const QString escaped = QStringLiteral("\"%1\"*").arg(token);
    return escaped;
}

/**
 * @brief Builds a conservative FTS5 expression from plain search text.
 * @param text User-entered plain text.
 * @param fields Selected searchable fields; empty means all indexed fields.
 * @return FTS5 expression, or an empty string for empty input.
 */
[[nodiscard]] auto create_fts_expression(const QString& text,
                                         const QSet<QString>& fields) -> QString
{
    const QStringList tokens =
        text.trimmed().split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
    QStringList escaped_tokens;
    for (const QString& token: tokens)
    {
        escaped_tokens.append(escape_fts_token(token));
    }

    QString expression = escaped_tokens.join(QStringLiteral(" AND "));
    QStringList columns;
    if (!fields.isEmpty())
    {
        if (fields.contains(LogField::Message))
        {
            columns.append(QStringLiteral("message"));
        }
        if (fields.contains(LogField::Level))
        {
            columns.append(QStringLiteral("level"));
        }
        if (fields.contains(LogField::AppName))
        {
            columns.append(QStringLiteral("app_name"));
        }
        if (!columns.isEmpty() && !expression.isEmpty())
        {
            expression =
                columns.size() == 1
                    ? QStringLiteral("%1 : %2").arg(columns.first(), expression)
                    : QStringLiteral("{%1} : %2").arg(columns.join(QLatin1Char(' ')), expression);
        }
    }
    return expression;
}

/**
 * @brief Resolves and validates every cache source belonging to a view.
 * @param catalog Catalog containing generation mappings.
 * @param views Registry containing files and profiles.
 * @param view_id View to resolve.
 * @return Ordered sources, or an empty vector when the mapping is incomplete or stale.
 */
[[nodiscard]] auto resolve_sources(LogCacheCatalog* catalog, ViewRegistry* views,
                                   const QUuid& view_id) -> QVector<CacheSource>
{
    QVector<CacheSource> sources;
    const LogViewContext* context = views != nullptr ? views->get_context(view_id) : nullptr;
    const QVector<qint64> generation_ids =
        catalog != nullptr ? catalog->get_view_generations(view_id) : QVector<qint64>();
    const QVector<QString> file_paths =
        context != nullptr ? context->get_file_paths() : QVector<QString>();
    const bool mapping_available = catalog != nullptr && catalog->is_available() &&
                                   context != nullptr && !file_paths.isEmpty() &&
                                   file_paths.size() == generation_ids.size();

    bool valid = mapping_available;
    for (qsizetype index = 0; index < generation_ids.size() && valid; ++index)
    {
        const QString file_path = QFileInfo(file_paths.at(index)).absoluteFilePath();
        const std::optional<LogParsingProfile> profile =
            context->get_file_parsing_profile(file_path);
        const std::optional<LogCacheGeneration> generation =
            catalog->get_generation(generation_ids.at(index));
        valid = profile.has_value() && generation.has_value() &&
                generation->state == LogCacheGenerationState::Complete &&
                QFileInfo::exists(generation->database_path);
        if (valid)
        {
            sources.append(CacheSource{generation.value(), profile.value(), file_path, index});
        }
    }

    if (!valid)
    {
        sources.clear();
    }
    return sources;
}

/**
 * @brief Determines whether a whole source participates in file-path filtering.
 * @param source Cache source under consideration.
 * @param query View query.
 * @return True when the source is not hidden and satisfies show-only selection.
 */
[[nodiscard]] auto source_is_visible(const CacheSource& source, const LogQuery& query) -> bool
{
    const QString absolute_path = QFileInfo(source.file_path).absoluteFilePath();
    const bool shown = query.show_only_file.isEmpty() ||
                       QFileInfo(query.show_only_file).absoluteFilePath() == absolute_path;
    const bool hidden =
        query.hidden_files.contains(absolute_path) || query.hidden_files.contains(source.file_path);
    return shown && !hidden;
}

/**
 * @brief Tests whether one materialized entry satisfies non-regex query filters.
 * @param entry Entry to inspect.
 * @param query View query.
 * @return True when file, application, level and plain-text filters match.
 */
[[nodiscard]] auto live_entry_matches(const LogEntry& entry, const LogQuery& query) -> bool
{
    const QString file_path = QFileInfo(entry.get_file_info().get_file_path()).absoluteFilePath();
    const bool shown = query.show_only_file.isEmpty() ||
                       QFileInfo(query.show_only_file).absoluteFilePath() == file_path;
    const bool hidden = query.hidden_files.contains(file_path) ||
                        query.hidden_files.contains(entry.get_file_info().get_file_path());
    const bool app_matches = query.app_name.isEmpty() || entry.get_app_name() == query.app_name;
    bool level_matches = query.log_levels.isEmpty();
    for (auto iterator = query.log_levels.cbegin();
         iterator != query.log_levels.cend() && !level_matches; ++iterator)
    {
        level_matches =
            iterator->trimmed().compare(entry.get_level().trimmed(), Qt::CaseInsensitive) == 0;
    }

    bool search_matches = query.search_text.trimmed().isEmpty() || query.use_regex;
    if (!search_matches)
    {
        const bool all_fields = query.search_fields.isEmpty();
        const QString text = query.search_text.trimmed();
        if (all_fields || query.search_fields.contains(LogField::Message))
        {
            search_matches = entry.get_message().contains(text, Qt::CaseInsensitive);
        }
        if (!search_matches && (all_fields || query.search_fields.contains(LogField::Level)))
        {
            search_matches = entry.get_level().contains(text, Qt::CaseInsensitive);
        }
        if (!search_matches && (all_fields || query.search_fields.contains(LogField::AppName)))
        {
            search_matches = entry.get_app_name().contains(text, Qt::CaseInsensitive);
        }
        if (!search_matches && (all_fields || query.search_fields.contains(LogField::FilePath)))
        {
            search_matches = file_path.contains(text, Qt::CaseInsensitive);
        }
    }

    const bool matches = shown && !hidden && app_matches && level_matches && search_matches;
    return matches;
}

/**
 * @brief Adds matching materialized live entries to collected cache rows.
 * @param rows Destination row collection.
 * @param entries Live entries belonging to the queried view.
 * @param file_paths Ordered source files registered on the view.
 * @param query View query.
 */
auto append_live_rows(QVector<CacheRow>& rows, const QVector<LogEntry>& entries,
                      const QVector<QString>& file_paths, const LogQuery& query) -> void
{
    for (qsizetype index = 0; index < entries.size(); ++index)
    {
        const LogEntry& entry = entries.at(index);
        if (live_entry_matches(entry, query))
        {
            const QString file_path =
                QFileInfo(entry.get_file_info().get_file_path()).absoluteFilePath();
            qsizetype source_position = file_paths.indexOf(file_path);
            if (source_position < 0)
            {
                source_position = file_paths.size();
            }
            CacheRow row;
            row.source.file_path = file_path;
            row.source.position = source_position;
            row.entry_id = std::numeric_limits<qint64>::max() - entries.size() + index;
            row.source_line = entry.get_source_line();
            row.byte_offset = entry.get_byte_offset();
            row.byte_length = entry.get_byte_length();
            row.timestamp = entry.get_timestamp();
            row.level = entry.get_level();
            row.app_name = entry.get_app_name();
            row.materialized_entry = entry;
            rows.append(std::move(row));
        }
    }
}

/**
 * @brief Materializes one cached row by parsing only its original source record.
 * @param row Row containing source byte coordinates and profile.
 * @return True when the source record was read and parsed successfully.
 */
auto materialize_row(CacheRow& row) -> bool
{
    bool materialized = row.materialized_entry.has_value();
    if (!materialized && row.byte_offset >= 0 && row.byte_length >= 0)
    {
        QFile file(row.source.file_path);
        materialized = file.open(QIODevice::ReadOnly) && file.seek(row.byte_offset);
        if (materialized)
        {
            const QByteArray record_bytes = file.read(row.byte_length);
            materialized = record_bytes.size() == row.byte_length;
            if (materialized)
            {
                const QString record = QString::fromUtf8(record_bytes);
                const LogParseOutcome outcome =
                    LogParser(row.source.profile.value())
                        .parse_line(record, row.source.file_path, row.source_line);
                materialized = outcome.succeeded();
                if (materialized)
                {
                    LogEntry entry = outcome.entry.value();
                    entry.set_source_range(row.byte_offset, row.byte_length);
                    row.materialized_entry = std::move(entry);
                }
            }
        }
    }
    return materialized;
}

/**
 * @brief Tests a materialized row against a regular expression and selected fields.
 * @param row Materialized cache row.
 * @param query Query containing the expression and field selection.
 * @return True when at least one selected value matches.
 */
[[nodiscard]] auto matches_regex(const CacheRow& row, const LogQuery& query) -> bool
{
    const QRegularExpression expression(query.search_text,
                                        QRegularExpression::CaseInsensitiveOption);
    const LogEntry& entry = row.materialized_entry.value();
    const bool all_fields = query.search_fields.isEmpty();
    bool matches = false;
    if (all_fields || query.search_fields.contains(LogField::Message))
    {
        matches = expression.match(entry.get_message()).hasMatch();
    }
    if (!matches && (all_fields || query.search_fields.contains(LogField::Level)))
    {
        matches = expression.match(entry.get_level()).hasMatch();
    }
    if (!matches && (all_fields || query.search_fields.contains(LogField::AppName)))
    {
        matches = expression.match(entry.get_app_name()).hasMatch();
    }
    if (!matches && (all_fields || query.search_fields.contains(LogField::FilePath)))
    {
        matches = expression.match(row.source.file_path).hasMatch();
    }
    return matches;
}

/** @brief SQL fragments and bindings shared by cache count and page queries. */
struct SourceSqlFilter {
        /** @brief Optional joins required by full-text search. */
        QString joins;
        /** @brief Optional WHERE clause including its leading keyword. */
        QString where;
        /** @brief Positional values bound to predicates in declaration order. */
        QList<QVariant> bindings;
        /** @brief Whether file visibility permits querying this source. */
        bool visible{false};
};

/**
 * @brief Builds reusable SQL filtering fragments for one cached source.
 * @param source Cache source under consideration.
 * @param query View query containing filters and plain-text search.
 * @return SQL fragments, bindings and source visibility.
 */
[[nodiscard]] auto create_source_sql_filter(const CacheSource& source,
                                            const LogQuery& query) -> SourceSqlFilter
{
    SourceSqlFilter filter;
    filter.visible = source_is_visible(source, query);
    if (filter.visible)
    {
        QStringList predicates;
        if (!query.app_name.isEmpty())
        {
            predicates.append(QStringLiteral("a.value=?"));
            filter.bindings.append(query.app_name);
        }
        if (!query.log_levels.isEmpty())
        {
            QStringList placeholders;
            for (const QString& level: query.log_levels)
            {
                placeholders.append(QStringLiteral("?"));
                filter.bindings.append(level.trimmed().toCaseFolded());
            }
            predicates.append(QStringLiteral("l.normalized_value IN (%1)")
                                  .arg(placeholders.join(QStringLiteral(","))));
        }

        const bool plain_search = !query.use_regex && !query.search_text.trimmed().isEmpty();
        const bool path_selected =
            query.search_fields.isEmpty() || query.search_fields.contains(LogField::FilePath);
        const bool path_matches =
            path_selected &&
            source.file_path.contains(query.search_text.trimmed(), Qt::CaseInsensitive);
        const bool indexed_field_selected = query.search_fields.isEmpty() ||
                                            query.search_fields.contains(LogField::Message) ||
                                            query.search_fields.contains(LogField::Level) ||
                                            query.search_fields.contains(LogField::AppName);
        const QString fts_expression =
            plain_search && indexed_field_selected
                ? create_fts_expression(query.search_text, query.search_fields)
                : QString();
        if (plain_search && !path_matches && !fts_expression.isEmpty())
        {
            filter.joins = QStringLiteral(" JOIN log_entries_fts f ON f.rowid=e.id");
            predicates.append(QStringLiteral("f.log_entries_fts MATCH ?"));
            filter.bindings.append(fts_expression);
        }
        else if (plain_search && !path_matches && !indexed_field_selected)
        {
            predicates.append(QStringLiteral("0"));
        }

        if (!predicates.isEmpty())
        {
            filter.where = QStringLiteral(" WHERE ") + predicates.join(QStringLiteral(" AND "));
        }
    }
    return filter;
}

/**
 * @brief Builds SQL ordering equivalent to the stable in-memory comparator.
 * @param query Query containing the selected sort field and direction.
 * @return ORDER BY clause for one source database.
 */
[[nodiscard]] auto create_source_order(const LogQuery& query) -> QString
{
    QString column = QStringLiteral("e.id");
    if (query.sort_field == LogField::Timestamp)
    {
        column = QStringLiteral("e.timestamp_utc_ms");
    }
    else if (query.sort_field == LogField::Level)
    {
        column = QStringLiteral("l.value COLLATE NOCASE");
    }
    else if (query.sort_field == LogField::AppName)
    {
        column = QStringLiteral("a.value COLLATE NOCASE");
    }
    const QString direction =
        query.sort_order == Qt::DescendingOrder ? QStringLiteral("DESC") : QStringLiteral("ASC");
    return QStringLiteral(" ORDER BY %1 %2, e.id %2").arg(column, direction);
}

/**
 * @brief Binds positional values to a prepared query.
 * @param sql_query Query receiving values.
 * @param bindings Values in placeholder order.
 */
auto bind_values(QSqlQuery& sql_query, const QList<QVariant>& bindings) -> void
{
    for (qsizetype index = 0; index < bindings.size(); ++index)
    {
        sql_query.bindValue(index, bindings.at(index));
    }
}

/**
 * @brief Loads filtered metadata rows from one cache database.
 * @param source Validated cache source.
 * @param query View query.
 * @param maximum_rows Maximum ordered rows to return, or -1 for every match.
 * @return Matching row metadata in the query's requested order.
 */
[[nodiscard]] auto load_source_rows(const CacheSource& source, const LogQuery& query,
                                    qsizetype maximum_rows = -1) -> QVector<CacheRow>
{
    QVector<CacheRow> rows;
    const SourceSqlFilter filter = create_source_sql_filter(source, query);
    if (filter.visible)
    {
        const QString connection_name =
            QStringLiteral("qt_log_viewer_cache_read_%1")
                .arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
        {
            QSqlDatabase database =
                QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connection_name);
            database.setDatabaseName(source.generation.database_path);
            database.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));
            if (database.open())
            {
                QString sql = QStringLiteral(
                    "SELECT e.id, e.source_line, e.byte_offset, e.byte_length, "
                    "e.timestamp_utc_ms, l.value, a.value "
                    "FROM log_entries e JOIN log_levels l ON l.id=e.level_id "
                    "JOIN applications a ON a.id=e.app_id");
                sql += filter.joins + filter.where + create_source_order(query);
                QList<QVariant> bindings = filter.bindings;
                if (maximum_rows >= 0)
                {
                    sql += QStringLiteral(" LIMIT ?");
                    bindings.append(maximum_rows);
                }

                QSqlQuery sql_query(database);
                sql_query.prepare(sql);
                bind_values(sql_query, bindings);
                if (sql_query.exec())
                {
                    while (sql_query.next())
                    {
                        CacheRow row;
                        row.source = source;
                        row.entry_id = sql_query.value(0).toLongLong();
                        row.source_line = sql_query.value(1).toLongLong();
                        row.byte_offset = sql_query.value(2).toLongLong();
                        row.byte_length = sql_query.value(3).toLongLong();
                        if (!sql_query.value(4).isNull())
                        {
                            row.timestamp = QDateTime::fromMSecsSinceEpoch(
                                sql_query.value(4).toLongLong(), QTimeZone::UTC);
                        }
                        row.level = sql_query.value(5).toString();
                        row.app_name = sql_query.value(6).toString();
                        rows.append(std::move(row));
                    }
                }
                database.close();
            }
        }
        QSqlDatabase::removeDatabase(connection_name);
    }
    return rows;
}

/**
 * @brief Counts matching rows inside one source database without materializing metadata.
 * @param source Validated cache source.
 * @param query View query containing SQL-compatible filters.
 * @return Matching row count, or zero when the source is hidden or unavailable.
 */
[[nodiscard]] auto count_source_rows(const CacheSource& source, const LogQuery& query) -> qsizetype
{
    const SourceSqlFilter filter = create_source_sql_filter(source, query);
    qsizetype count = 0;
    if (filter.visible)
    {
        const QString connection_name =
            QStringLiteral("qt_log_viewer_cache_count_%1")
                .arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
        {
            QSqlDatabase database =
                QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connection_name);
            database.setDatabaseName(source.generation.database_path);
            database.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));
            if (database.open())
            {
                QSqlQuery sql_query(database);
                sql_query.prepare(QStringLiteral("SELECT COUNT(*) FROM log_entries e "
                                                 "JOIN log_levels l ON l.id=e.level_id "
                                                 "JOIN applications a ON a.id=e.app_id") +
                                  filter.joins + filter.where);
                bind_values(sql_query, filter.bindings);
                if (sql_query.exec() && sql_query.next())
                {
                    count = sql_query.value(0).toLongLong();
                }
                database.close();
            }
        }
        QSqlDatabase::removeDatabase(connection_name);
    }
    return count;
}

/**
 * @brief Groups matching rows by normalized level inside one source database.
 * @param source Validated cache source.
 * @param query Query whose level selection has already been cleared.
 * @return Uppercase level names and counts for this source.
 */
[[nodiscard]] auto get_source_level_counts(const CacheSource& source,
                                           const LogQuery& query) -> QMap<QString, qsizetype>
{
    const SourceSqlFilter filter = create_source_sql_filter(source, query);
    QMap<QString, qsizetype> counts;
    if (filter.visible)
    {
        const QString connection_name =
            QStringLiteral("qt_log_viewer_cache_facets_%1")
                .arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
        {
            QSqlDatabase database =
                QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connection_name);
            database.setDatabaseName(source.generation.database_path);
            database.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));
            if (database.open())
            {
                QSqlQuery sql_query(database);
                sql_query.prepare(QStringLiteral("SELECT l.value, COUNT(*) FROM log_entries e "
                                                 "JOIN log_levels l ON l.id=e.level_id "
                                                 "JOIN applications a ON a.id=e.app_id") +
                                  filter.joins + filter.where +
                                  QStringLiteral(" GROUP BY l.normalized_value"));
                bind_values(sql_query, filter.bindings);
                if (sql_query.exec())
                {
                    while (sql_query.next())
                    {
                        const QString level = sql_query.value(0).toString().trimmed().toUpper();
                        if (!level.isEmpty())
                        {
                            counts[level] += sql_query.value(1).toLongLong();
                        }
                    }
                }
                database.close();
            }
        }
        QSqlDatabase::removeDatabase(connection_name);
    }
    return counts;
}

/**
 * @brief Collects matching metadata rows from all files and applies regex filtering if required.
 * @param sources Validated cache sources.
 * @param query View query.
 * @return Globally collected matching rows.
 */
[[nodiscard]] auto collect_rows(const QVector<CacheSource>& sources,
                                const QVector<LogEntry>& live_entries,
                                const QVector<QString>& file_paths,
                                const LogQuery& query) -> QVector<CacheRow>
{
    QVector<CacheRow> rows;
    for (const CacheSource& source: sources)
    {
        rows.append(load_source_rows(source, query));
    }
    append_live_rows(rows, live_entries, file_paths, query);

    if (query.use_regex && !query.search_text.trimmed().isEmpty())
    {
        const QRegularExpression expression(query.search_text,
                                            QRegularExpression::CaseInsensitiveOption);
        QVector<CacheRow> matching_rows;
        if (expression.isValid())
        {
            for (CacheRow& row: rows)
            {
                if (materialize_row(row) && matches_regex(row, query))
                {
                    matching_rows.append(std::move(row));
                }
            }
        }
        rows = std::move(matching_rows);
    }
    return rows;
}

/**
 * @brief Compares two cache rows using stable query ordering.
 * @param left First row.
 * @param right Second row.
 * @param query Sorting state.
 * @return True when left precedes right.
 */
[[nodiscard]] auto row_precedes(const CacheRow& left, const CacheRow& right,
                                const LogQuery& query) -> bool
{
    int comparison = 0;
    if (query.sort_field == LogField::Timestamp)
    {
        comparison =
            left.timestamp < right.timestamp ? -1 : (left.timestamp > right.timestamp ? 1 : 0);
    }
    else if (query.sort_field == LogField::Level)
    {
        comparison = QString::compare(left.level, right.level, Qt::CaseInsensitive);
    }
    else if (query.sort_field == LogField::AppName)
    {
        comparison = QString::compare(left.app_name, right.app_name, Qt::CaseInsensitive);
    }
    else if (query.sort_field == LogField::FilePath)
    {
        comparison =
            QString::compare(left.source.file_path, right.source.file_path, Qt::CaseInsensitive);
    }
    else if (query.sort_field == LogField::Message)
    {
        comparison = QString::compare(left.materialized_entry->get_message(),
                                      right.materialized_entry->get_message(), Qt::CaseInsensitive);
    }
    if (comparison == 0)
    {
        comparison = left.source.position < right.source.position
                         ? -1
                         : (left.source.position > right.source.position ? 1 : 0);
    }
    if (comparison == 0)
    {
        comparison = left.entry_id < right.entry_id ? -1 : (left.entry_id > right.entry_id ? 1 : 0);
    }
    const bool precedes = query.sort_order == Qt::DescendingOrder ? comparison > 0 : comparison < 0;
    return precedes;
}
}  // namespace

/**
 * @brief Constructs the cache reader.
 * @param catalog Catalog resolving view mappings.
 * @param views Registry providing file and parser state.
 */
LogCacheReadService::LogCacheReadService(LogCacheCatalog* catalog, ViewRegistry* views)
    : m_catalog(catalog), m_views(views)
{}

/**
 * @brief Validates that every registered source has an ordered complete cache generation.
 * @param view_id View whose current generation mapping is inspected.
 * @return True when cache-backed queries can represent the complete view.
 */
auto LogCacheReadService::can_query(const QUuid& view_id) const -> bool
{
    const bool queryable = !resolve_sources(m_catalog, m_views, view_id).isEmpty();
    return queryable;
}

/**
 * @brief Counts cache rows and live-overlay entries matching a complete query.
 * @param query View, search and filter state to apply.
 * @return Number of matching entries across all bound sources.
 */
auto LogCacheReadService::count_entries(const LogQuery& query) const -> qsizetype
{
    const QVector<CacheSource> sources = resolve_sources(m_catalog, m_views, query.view_id);
    const QVector<QString> file_paths =
        m_views != nullptr ? m_views->get_file_paths(query.view_id) : QVector<QString>();
    const QVector<LogEntry> live_entries = m_live_entries.value(query.view_id);
    const bool requires_materialization = query.use_regex && !query.search_text.trimmed().isEmpty();
    qsizetype count = 0;
    if (requires_materialization)
    {
        count = collect_rows(sources, live_entries, file_paths, query).size();
    }
    else
    {
        for (const CacheSource& source: sources)
        {
            count += count_source_rows(source, query);
        }
        for (const LogEntry& entry: live_entries)
        {
            if (live_entry_matches(entry, query))
            {
                ++count;
            }
        }
    }
    return count;
}

/**
 * @brief Loads one globally ordered result page from all bound cache generations.
 * @param query View, search, filter and sorting state to apply.
 * @param offset Zero-based offset inside the merged result set.
 * @param limit Maximum number of entries to materialize.
 * @return Parsed entries for the requested page, including matching live-tail entries.
 */
auto LogCacheReadService::load_entries_page(const LogQuery& query, qsizetype offset,
                                            qsizetype limit) const -> QVector<LogEntry>
{
    QVector<LogEntry> entries;
    const QVector<QString> file_paths =
        m_views != nullptr ? m_views->get_file_paths(query.view_id) : QVector<QString>();
    const QVector<CacheSource> sources = resolve_sources(m_catalog, m_views, query.view_id);
    const QVector<LogEntry> live_entries = m_live_entries.value(query.view_id);
    const bool requires_materialization =
        query.sort_field == LogField::Message ||
        (query.use_regex && !query.search_text.trimmed().isEmpty());
    QVector<CacheRow> rows;
    if (requires_materialization)
    {
        rows = collect_rows(sources, live_entries, file_paths, query);
    }
    else
    {
        const qsizetype maximum_rows =
            std::max<qsizetype>(0, offset) + std::max<qsizetype>(0, limit);
        for (const CacheSource& source: sources)
        {
            rows.append(load_source_rows(source, query, maximum_rows));
        }
        append_live_rows(rows, live_entries, file_paths, query);
    }
    if (query.sort_field == LogField::Message)
    {
        for (CacheRow& row: rows)
        {
            materialize_row(row);
        }
        rows.erase(
            std::remove_if(rows.begin(), rows.end(),
                           [](const CacheRow& row) { return !row.materialized_entry.has_value(); }),
            rows.end());
    }
    std::stable_sort(rows.begin(), rows.end(),
                     [&query](const CacheRow& left, const CacheRow& right) {
                         return row_precedes(left, right, query);
                     });

    const qsizetype first = std::clamp<qsizetype>(offset, 0, rows.size());
    const qsizetype last =
        std::clamp<qsizetype>(first + std::max<qsizetype>(0, limit), first, rows.size());
    entries.reserve(last - first);
    for (qsizetype index = first; index < last; ++index)
    {
        CacheRow& row = rows[index];
        if (materialize_row(row))
        {
            entries.append(row.materialized_entry.value());
        }
    }
    return entries;
}

/**
 * @brief Builds the level facet from cached metadata and the live overlay.
 * @param query Query whose selected level filter is intentionally ignored.
 * @return Uppercase level names mapped to their matching entry counts.
 */
auto LogCacheReadService::get_log_level_counts(const LogQuery& query) const
    -> QMap<QString, qsizetype>
{
    LogQuery facet_query = query;
    facet_query.log_levels.clear();
    QMap<QString, qsizetype> counts;
    const QVector<CacheSource> sources = resolve_sources(m_catalog, m_views, query.view_id);
    const QVector<LogEntry> live_entries = m_live_entries.value(query.view_id);
    const QVector<QString> file_paths =
        m_views != nullptr ? m_views->get_file_paths(query.view_id) : QVector<QString>();
    const bool requires_materialization =
        facet_query.use_regex && !facet_query.search_text.trimmed().isEmpty();
    if (requires_materialization)
    {
        const QVector<CacheRow> rows = collect_rows(sources, live_entries, file_paths, facet_query);
        for (const CacheRow& row: rows)
        {
            const QString level = row.level.trimmed().toUpper();
            if (!level.isEmpty())
            {
                counts[level] += 1;
            }
        }
    }
    else
    {
        for (const CacheSource& source: sources)
        {
            const QMap<QString, qsizetype> source_counts =
                get_source_level_counts(source, facet_query);
            for (auto iterator = source_counts.cbegin(); iterator != source_counts.cend();
                 ++iterator)
            {
                counts[iterator.key()] += iterator.value();
            }
        }
        for (const LogEntry& entry: live_entries)
        {
            if (live_entry_matches(entry, facet_query))
            {
                const QString level = entry.get_level().trimmed().toUpper();
                if (!level.isEmpty())
                {
                    counts[level] += 1;
                }
            }
        }
    }
    return counts;
}

/**
 * @brief Collects distinct values for one supported standard field.
 * @param view_id View whose complete cache mapping is queried.
 * @param field_id Stable application, level or file-path field identifier.
 * @return Distinct non-empty values from cached and live entries.
 */
auto LogCacheReadService::get_distinct_values(const QUuid& view_id,
                                              const QString& field_id) const -> QSet<QString>
{
    QSet<QString> values;
    const QVector<CacheSource> sources = resolve_sources(m_catalog, m_views, view_id);
    for (const CacheSource& source: sources)
    {
        if (field_id == LogField::FilePath)
        {
            values.insert(source.file_path);
        }
        else if (field_id == LogField::AppName || field_id == LogField::Level)
        {
            const QString connection_name =
                QStringLiteral("qt_log_viewer_cache_distinct_%1")
                    .arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
            {
                QSqlDatabase database =
                    QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), connection_name);
                database.setDatabaseName(source.generation.database_path);
                database.setConnectOptions(QStringLiteral("QSQLITE_OPEN_READONLY"));
                if (database.open())
                {
                    const QString sql = field_id == LogField::AppName
                                            ? QStringLiteral(
                                                  "SELECT DISTINCT a.value FROM log_entries e "
                                                  "JOIN applications a ON a.id=e.app_id "
                                                  "WHERE a.value<>''")
                                            : QStringLiteral(
                                                  "SELECT DISTINCT l.value FROM log_entries e "
                                                  "JOIN log_levels l ON l.id=e.level_id "
                                                  "WHERE l.value<>''");
                    QSqlQuery sql_query(database);
                    if (sql_query.exec(sql))
                    {
                        while (sql_query.next())
                        {
                            values.insert(sql_query.value(0).toString());
                        }
                    }
                    database.close();
                }
            }
            QSqlDatabase::removeDatabase(connection_name);
        }
    }

    const QVector<LogEntry> live_entries = m_live_entries.value(view_id);
    for (const LogEntry& entry: live_entries)
    {
        QString value;
        if (field_id == LogField::AppName)
        {
            value = entry.get_app_name();
        }
        else if (field_id == LogField::Level)
        {
            value = entry.get_level();
        }
        else if (field_id == LogField::FilePath)
        {
            value = QFileInfo(entry.get_file_info().get_file_path()).absoluteFilePath();
        }
        if (!value.isEmpty())
        {
            values.insert(value);
        }
    }
    return values;
}

/**
 * @brief Appends newly tailed entries to a cache-backed view's in-memory overlay.
 * @param view_id View receiving the parsed live records.
 * @param entries Entries to expose above its immutable cache generations.
 */
auto LogCacheReadService::append_live_entries(const QUuid& view_id,
                                              const QVector<LogEntry>& entries) -> void
{
    if (!view_id.isNull() && !entries.isEmpty() && can_query(view_id))
    {
        m_live_entries[view_id].append(entries);
    }
}

/**
 * @brief Discards every live-overlay entry associated with a view.
 * @param view_id View whose transient entries are removed.
 */
auto LogCacheReadService::remove_view(const QUuid& view_id) -> void
{
    m_live_entries.remove(view_id);
}

/**
 * @brief Discards live-overlay entries originating from one source file.
 * @param view_id View owning the transient entries.
 * @param file_path Source file whose entries are removed.
 */
auto LogCacheReadService::remove_file(const QUuid& view_id, const QString& file_path) -> void
{
    auto iterator = m_live_entries.find(view_id);
    if (iterator != m_live_entries.end())
    {
        const QString absolute_file_path = QFileInfo(file_path).absoluteFilePath();
        iterator->erase(
            std::remove_if(
                iterator->begin(), iterator->end(),
                [&absolute_file_path](const LogEntry& entry) {
                    return QFileInfo(entry.get_file_info().get_file_path()).absoluteFilePath() ==
                           absolute_file_path;
                }),
            iterator->end());
        if (iterator->isEmpty())
        {
            m_live_entries.erase(iterator);
        }
    }
}
