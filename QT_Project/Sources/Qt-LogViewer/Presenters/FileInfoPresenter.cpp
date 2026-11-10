/**
 * @file FileInfoPresenter.cpp
 * @brief Implements aggregation of source metadata for the file inspector.
 */

#include "Qt-LogViewer/Presenters/FileInfoPresenter.h"

#include <QFile>
#include <QFileInfo>
#include <QLocale>

#include "Qt-LogViewer/Controllers/LiveTailingCoordinator.h"
#include "Qt-LogViewer/Controllers/LogViewContext.h"
#include "Qt-LogViewer/Controllers/ViewRegistry.h"
#include "Qt-LogViewer/Models/LogFieldDefinition.h"
#include "Qt-LogViewer/Models/LogQuery.h"
#include "Qt-LogViewer/Services/LogCacheCatalog.h"
#include "Qt-LogViewer/Services/LogHistoryService.h"
#include "Qt-LogViewer/Views/App/FileInfoWidget.h"

namespace
{
/**
 * @brief Normalizes a source path for case-safe metadata lookup.
 * @param file_path Source path to normalize.
 * @return Absolute path, case-folded on Windows.
 */
[[nodiscard]] auto normalized_path_key(const QString& file_path) -> QString
{
    QString key = QFileInfo(file_path).absoluteFilePath();
#ifdef Q_OS_WIN
    key = key.toCaseFolded();
#endif
    return key;
}

/**
 * @brief Formats a byte count for the current locale.
 * @param bytes Non-negative byte count.
 * @return Human-readable IEC size.
 */
[[nodiscard]] auto format_bytes(qint64 bytes) -> QString
{
    const QLocale locale;
    QString text;
    if (bytes >= 1024LL * 1024LL * 1024LL)
    {
        text = locale.toString(static_cast<double>(bytes) / (1024.0 * 1024.0 * 1024.0), 'f', 2) +
               QStringLiteral(" GiB");
    }
    else if (bytes >= 1024LL * 1024LL)
    {
        text = locale.toString(static_cast<double>(bytes) / (1024.0 * 1024.0), 'f', 2) +
               QStringLiteral(" MiB");
    }
    else if (bytes >= 1024LL)
    {
        text =
            locale.toString(static_cast<double>(bytes) / 1024.0, 'f', 2) + QStringLiteral(" KiB");
    }
    else
    {
        text = locale.toString(bytes) + QStringLiteral(" B");
    }
    return text;
}

/**
 * @brief Formats a timestamp in local time.
 * @param value UTC or local timestamp.
 * @return Localized timestamp, or a placeholder for an invalid value.
 */
[[nodiscard]] auto format_time(const QDateTime& value) -> QString
{
    const QString text = value.isValid()
                             ? QLocale().toString(value.toLocalTime(), QLocale::ShortFormat)
                             : QObject::tr("Not available");
    return text;
}

/**
 * @brief Inspects a bounded source prefix for a byte-order mark.
 * @param file_path Source file path.
 * @return Safely identified encoding or an explicit unknown value.
 */
[[nodiscard]] auto detect_encoding(const QString& file_path) -> QString
{
    QFile file(file_path);
    QByteArray prefix;
    if (file.open(QIODevice::ReadOnly))
    {
        prefix = file.read(4);
    }
    QString encoding = QObject::tr("Not safely detected");
    if (prefix.startsWith("\xEF\xBB\xBF"))
    {
        encoding = QStringLiteral("UTF-8 BOM");
    }
    else if (prefix.startsWith("\xFF\xFE\x00\x00"))
    {
        encoding = QStringLiteral("UTF-32 LE BOM");
    }
    else if (prefix.startsWith("\x00\x00\xFE\xFF"))
    {
        encoding = QStringLiteral("UTF-32 BE BOM");
    }
    else if (prefix.startsWith("\xFF\xFE"))
    {
        encoding = QStringLiteral("UTF-16 LE BOM");
    }
    else if (prefix.startsWith("\xFE\xFF"))
    {
        encoding = QStringLiteral("UTF-16 BE BOM");
    }
    return encoding;
}

/**
 * @brief Inspects a bounded source prefix for unambiguous newline bytes.
 * @param file_path Source file path.
 * @return Detected convention, mixed marker, or unknown value.
 */
[[nodiscard]] auto detect_line_endings(const QString& file_path) -> QString
{
    QFile file(file_path);
    QByteArray sample;
    if (file.open(QIODevice::ReadOnly))
    {
        sample = file.read(64 * 1024);
    }
    const bool has_crlf = sample.contains("\r\n");
    QByteArray without_crlf = sample;
    without_crlf.replace("\r\n", "");
    const bool has_lf = without_crlf.contains('\n');
    const bool has_cr = without_crlf.contains('\r');
    QString result = QObject::tr("Not safely detected");
    const int convention_count =
        static_cast<int>(has_crlf) + static_cast<int>(has_lf) + static_cast<int>(has_cr);
    if (convention_count > 1)
    {
        result = QObject::tr("Mixed");
    }
    else if (has_crlf)
    {
        result = QStringLiteral("CRLF");
    }
    else if (has_lf)
    {
        result = QStringLiteral("LF");
    }
    else if (has_cr)
    {
        result = QStringLiteral("CR");
    }
    return result;
}

/**
 * @brief Joins sorted values for compact display.
 * @param values Values to sort and join.
 * @return Comma-separated values or a placeholder.
 */
[[nodiscard]] auto join_values(const QSet<QString>& values) -> QString
{
    QStringList sorted(values.begin(), values.end());
    sorted.sort(Qt::CaseInsensitive);
    const QString result =
        sorted.isEmpty() ? QObject::tr("None detected") : sorted.join(QStringLiteral(", "));
    return result;
}
}  // namespace

/**
 * @brief Constructs and connects the file-information presenter.
 * @param views Runtime view registry.
 * @param history Archived entry query service.
 * @param cache_catalog Persistent cache metadata catalog.
 * @param live_tailing Live-tail state owner.
 * @param widget Inspector widget receiving snapshots.
 * @param parent Optional QObject parent.
 */
FileInfoPresenter::FileInfoPresenter(ViewRegistry* views, LogHistoryService* history,
                                     LogCacheCatalog* cache_catalog,
                                     LiveTailingCoordinator* live_tailing, FileInfoWidget* widget,
                                     QObject* parent)
    : QObject(parent),
      m_views(views),
      m_history(history),
      m_cache_catalog(cache_catalog),
      m_live_tailing(live_tailing),
      m_widget(widget)
{
    connect(m_views, &ViewRegistry::current_view_id_changed, this, [this](const QUuid& view_id) {
        m_active_view_id = view_id;
        m_selected_index = 0;
        refresh();
    });
    connect(m_views, &ViewRegistry::view_file_paths_changed, this,
            [this](const QUuid& view_id, const QVector<QString>&) {
                if (view_id == m_active_view_id)
                {
                    m_selected_index = 0;
                    refresh();
                }
            });
    connect(m_live_tailing, &LiveTailingCoordinator::live_tail_updated, this,
            [this](const QUuid& view_id, const QString&, const QDateTime&) {
                if (view_id == m_active_view_id)
                {
                    refresh();
                }
            });
    connect(m_widget, &FileInfoWidget::snapshot_selected, this,
            [this](qsizetype index) { m_selected_index = index; });
    m_active_view_id = m_views->get_current_view();
    refresh();
}

/** @brief Rebuilds aggregate and per-file snapshots for the active view. */
auto FileInfoPresenter::refresh() -> void
{
    QVector<FileInfoSnapshot> snapshots;
    const QVector<QString> file_paths = m_views->get_file_paths(m_active_view_id);
    LogViewContext* context = m_views->get_context(m_active_view_id);
    const QVector<qint64> generation_ids = m_cache_catalog->get_view_generations(m_active_view_id);
    QHash<QString, LogCacheGeneration> generations;
    for (qint64 generation_id: generation_ids)
    {
        const std::optional<LogCacheGeneration> generation =
            m_cache_catalog->get_generation(generation_id);
        if (generation.has_value())
        {
            generations.insert(normalized_path_key(generation->identity.canonical_file_path),
                               generation.value());
        }
    }

    qint64 aggregate_size = 0;
    qint64 aggregate_cache_size = 0;
    qsizetype aggregate_records = 0;
    QDateTime aggregate_earliest;
    QDateTime aggregate_latest;
    QSet<QString> aggregate_levels;
    QDateTime aggregate_cache_access;
    QDateTime aggregate_last_live_entry;

    for (const QString& file_path: file_paths)
    {
        const QFileInfo file_info(file_path);
        FileInfoSnapshot snapshot;
        snapshot.display_name = file_info.fileName();
        snapshot.path = file_info.absoluteFilePath();
        snapshot.local_path_available = file_info.exists();
        snapshot.size = file_info.exists() ? format_bytes(file_info.size()) : tr("Unavailable");
        snapshot.modified = format_time(file_info.lastModified());
        aggregate_size += file_info.exists() ? file_info.size() : 0;

        LogQuery query;
        query.view_id = m_active_view_id;
        query.show_only_file = file_info.absoluteFilePath();
        const qsizetype record_count = m_history->count_entries(query);
        aggregate_records += record_count;
        snapshot.record_count = QLocale().toString(record_count);

        query.sort_field = LogField::Timestamp;
        query.sort_order = Qt::AscendingOrder;
        const QVector<LogEntry> first_entries = m_history->load_entries_page(query, 0, 1);
        query.sort_order = Qt::DescendingOrder;
        const QVector<LogEntry> last_entries = m_history->load_entries_page(query, 0, 1);
        const QDateTime earliest =
            first_entries.isEmpty() ? QDateTime() : first_entries.constFirst().get_timestamp();
        const QDateTime latest =
            last_entries.isEmpty() ? QDateTime() : last_entries.constFirst().get_timestamp();
        snapshot.timestamp_range =
            format_time(earliest) + QStringLiteral(" – ") + format_time(latest);
        if (!aggregate_earliest.isValid() || (earliest.isValid() && earliest < aggregate_earliest))
        {
            aggregate_earliest = earliest;
        }
        if (!aggregate_latest.isValid() || (latest.isValid() && latest > aggregate_latest))
        {
            aggregate_latest = latest;
        }

        const QMap<QString, qsizetype> level_counts = m_history->get_log_level_counts(query);
        const QSet<QString> levels(level_counts.keyBegin(), level_counts.keyEnd());
        snapshot.log_levels = join_values(levels);
        aggregate_levels.unite(levels);
        snapshot.applications = tr("View: %1")
                                    .arg(join_values(m_history->get_distinct_values(
                                        m_active_view_id, LogField::AppName)));

        const std::optional<LogParsingProfile> profile =
            context != nullptr ? context->get_file_parsing_profile(file_info.absoluteFilePath())
                               : std::nullopt;
        snapshot.parsing_profile = profile.has_value() ? profile->get_name() : tr("Not available");
        snapshot.parse_success_rate = tr("Not captured during import");
        snapshot.encoding = detect_encoding(file_info.absoluteFilePath());
        snapshot.line_endings = detect_line_endings(file_info.absoluteFilePath());

        const auto generation =
            generations.constFind(normalized_path_key(file_info.absoluteFilePath()));
        if (generation != generations.cend())
        {
            snapshot.cache_state = generation->state == LogCacheGenerationState::Complete
                                       ? tr("Current")
                                       : tr("Incomplete");
            snapshot.cache_size = format_bytes(generation->storage_bytes);
            const QDateTime cache_access =
                QDateTime::fromMSecsSinceEpoch(generation->last_access_utc_ms, Qt::UTC);
            snapshot.cache_last_access = format_time(cache_access);
            aggregate_cache_size += generation->storage_bytes;
            if (!aggregate_cache_access.isValid() || cache_access > aggregate_cache_access)
            {
                aggregate_cache_access = cache_access;
            }
        }
        else
        {
            snapshot.cache_state = tr("Not cached");
            snapshot.cache_size = QStringLiteral("0 B");
            snapshot.cache_last_access = tr("Not available");
        }

        const bool tail_enabled = m_live_tailing->is_enabled(m_active_view_id);
        snapshot.live_tail_state = tail_enabled ? tr("Enabled") : tr("Disabled");
        const QDateTime last_live_entry =
            m_live_tailing->get_last_entry_time(m_active_view_id, file_info.absoluteFilePath());
        snapshot.last_live_entry = format_time(last_live_entry);
        if (!aggregate_last_live_entry.isValid() || last_live_entry > aggregate_last_live_entry)
        {
            aggregate_last_live_entry = last_live_entry;
        }
        snapshots.append(snapshot);
    }

    if (snapshots.size() > 1)
    {
        FileInfoSnapshot aggregate;
        aggregate.display_name = tr("All files (%1)").arg(snapshots.size());
        aggregate.path = tr("Multiple sources");
        aggregate.size = format_bytes(aggregate_size);
        aggregate.modified = tr("See individual files");
        aggregate.record_count = QLocale().toString(aggregate_records);
        aggregate.timestamp_range =
            format_time(aggregate_earliest) + QStringLiteral(" – ") + format_time(aggregate_latest);
        aggregate.applications =
            join_values(m_history->get_distinct_values(m_active_view_id, LogField::AppName));
        aggregate.log_levels = join_values(aggregate_levels);
        aggregate.parsing_profile = tr("See individual files");
        aggregate.parse_success_rate = tr("Not captured during import");
        aggregate.encoding = tr("See individual files");
        aggregate.line_endings = tr("See individual files");
        aggregate.cache_state =
            generations.size() == file_paths.size() ? tr("All cached") : tr("Partially cached");
        aggregate.cache_size = format_bytes(aggregate_cache_size);
        aggregate.cache_last_access = format_time(aggregate_cache_access);
        aggregate.live_tail_state =
            m_live_tailing->is_enabled(m_active_view_id) ? tr("Enabled for view") : tr("Disabled");
        aggregate.last_live_entry = format_time(aggregate_last_live_entry);
        snapshots.prepend(aggregate);
    }

    if (snapshots.isEmpty())
    {
        m_widget->clear();
    }
    else
    {
        m_widget->set_snapshots(snapshots, m_selected_index);
    }
}
