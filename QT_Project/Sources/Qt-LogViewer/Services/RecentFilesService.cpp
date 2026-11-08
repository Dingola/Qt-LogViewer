/**
 * @file RecentFilesService.cpp
 * @brief Implements recent-file MRU behavior and compatible index persistence.
 */

#include "Qt-LogViewer/Services/RecentFilesService.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>

#include "Qt-LogViewer/Services/SessionRepository.h"
#include "QtCommonLib/FileSystem/LocalFileStatus.h"

namespace
{
constexpr auto k_recent_files_key = "recent_files";
constexpr auto k_file_path_key = "file_path";
constexpr auto k_app_name_key = "app_name";
constexpr auto k_last_opened_key = "last_opened";

/**
 * @brief Compares normalized path keys using platform filesystem semantics.
 * @param left First normalized path key.
 * @param right Second normalized path key.
 * @return True when both keys identify the same platform path; otherwise false.
 */
[[nodiscard]] auto path_keys_equal(const QString& left, const QString& right) -> bool
{
#ifdef Q_OS_WIN
    const bool result = left.compare(right, Qt::CaseInsensitive) == 0;
#else
    const bool result = left == right;
#endif
    return result;
}

/**
 * @brief Parses an ISO 8601 timestamp accepted by the existing index schema.
 * @param value Persisted timestamp text.
 * @return Parsed timestamp, or an invalid value when parsing fails.
 */
[[nodiscard]] auto parse_iso_datetime(const QString& value) -> QDateTime
{
    QDateTime result = QDateTime::fromString(value, Qt::ISODateWithMs);
    if (!result.isValid())
    {
        result = QDateTime::fromString(value, Qt::ISODate);
    }
    return result;
}
}  // namespace

/**
 * @brief Constructs a recent-files service.
 * @param repository Repository containing the shared index document.
 * @param maximum_size Maximum retained entry count, or zero when unbounded.
 * @param parent Optional QObject parent.
 */
RecentFilesService::RecentFilesService(SessionRepository* repository, qsizetype maximum_size,
                                       QObject* parent)
    : QObject(parent),
      m_repository(repository),
      m_recent_files([](const RecentLogFileRecord& record) { return path_key(record.file_path); },
                     maximum_size,
                     [](const QString& left, const QString& right) {
                         const bool result = path_keys_equal(left, right);
                         return result;
                     })
{}

/**
 * @brief Loads existing recent-file records from the unchanged index schema.
 */
auto RecentFilesService::initialize_from_storage() -> void
{
    QVector<RecentLogFileRecord> records;
    if (m_repository != nullptr)
    {
        const QJsonArray files =
            m_repository->load_all().value(QString::fromLatin1(k_recent_files_key)).toArray();
        records.reserve(files.size());
        for (const QJsonValue& value: files)
        {
            records.append(from_json(value.toObject()));
        }
    }
    m_recent_files.assign(std::move(records));
    emit recent_log_files_changed(m_recent_files.items());
}

/**
 * @brief Returns recent files in most-recently-used order.
 * @return Copy of the current recent-file records.
 */
auto RecentFilesService::get_recent_log_files() const -> QVector<RecentLogFileRecord>
{
    QVector<RecentLogFileRecord> result = m_recent_files.items();
    return result;
}

/**
 * @brief Adds or updates a recent log file and persists the resulting order.
 * @param file_info File metadata used to create the recent-file record.
 */
auto RecentFilesService::add_recent_log_file(const LogFileInfo& file_info) -> void
{
    const QtCommonLib::LocalFileStatus status =
        QtCommonLib::inspect_local_file(file_info.get_file_path());
    const QString file_path =
        status.normalized_path.isEmpty() ? file_info.get_file_path() : status.normalized_path;
    m_recent_files.insert_or_update(
        {file_path, file_info.get_app_name(), QDateTime::currentDateTime()});
    persist();
    emit recent_log_files_changed(m_recent_files.items());
}

/**
 * @brief Clears all recent-file metadata and persists the empty list.
 */
auto RecentFilesService::clear_recent_log_files() -> void
{
    m_recent_files.clear();
    persist();
    emit recent_log_files_changed(m_recent_files.items());
}

/**
 * @brief Removes matching recent-file metadata without deleting source files.
 * @param file_paths Paths identifying entries to remove.
 * @return Number of removed entries.
 */
auto RecentFilesService::remove_recent_log_files(const QSet<QString>& file_paths) -> int
{
    int removed_count = 0;
    for (const QString& file_path: file_paths)
    {
        if (m_recent_files.remove(path_key(file_path)))
        {
            ++removed_count;
        }
    }
    if (removed_count > 0)
    {
        persist();
        emit recent_log_files_changed(m_recent_files.items());
    }
    return removed_count;
}

/**
 * @brief Replaces one recent-file path while preserving its remaining metadata.
 * @param previous_file_path Path identifying the existing entry.
 * @param replacement_file_path Readable replacement path selected by the user.
 * @return True when the entry changed and was persisted; otherwise false.
 */
auto RecentFilesService::relink_recent_log_file(const QString& previous_file_path,
                                                const QString& replacement_file_path) -> bool
{
    const QtCommonLib::LocalFileStatus replacement_status =
        QtCommonLib::inspect_local_file(replacement_file_path);
    const QString previous_key = path_key(previous_file_path);
    const QString replacement_key = path_key(replacement_status.normalized_path);
    bool changed =
        replacement_status.availability == QtCommonLib::LocalFileAvailability::Available &&
        !path_keys_equal(previous_key, replacement_key) && m_recent_files.contains(previous_key);

    if (changed)
    {
        QVector<RecentLogFileRecord> records;
        records.reserve(m_recent_files.size());
        for (const RecentLogFileRecord& record: m_recent_files.items())
        {
            const QString record_key = path_key(record.file_path);
            if (path_keys_equal(record_key, previous_key))
            {
                RecentLogFileRecord replacement = record;
                replacement.file_path = replacement_status.normalized_path;
                records.append(std::move(replacement));
            }
            else if (!path_keys_equal(record_key, replacement_key))
            {
                records.append(record);
            }
        }
        m_recent_files.assign(std::move(records));
        persist();
        emit recent_log_files_changed(m_recent_files.items());
    }
    return changed;
}

/**
 * @brief Produces a platform-appropriate stable comparison key for a path.
 * @param file_path Persisted or incoming local path.
 * @return Clean absolute path key.
 */
auto RecentFilesService::path_key(const QString& file_path) -> QString
{
    QString result;
    if (!file_path.isEmpty())
    {
        result = QDir::cleanPath(QFileInfo(file_path).absoluteFilePath());
    }
    return result;
}

/**
 * @brief Converts a recent-file record to its persisted JSON representation.
 * @param record Record to convert.
 * @return JSON object using the existing index schema.
 */
auto RecentFilesService::to_json(const RecentLogFileRecord& record) -> QJsonObject
{
    QJsonObject result;
    result.insert(QString::fromLatin1(k_file_path_key), record.file_path);
    result.insert(QString::fromLatin1(k_app_name_key), record.app_name);
    result.insert(QString::fromLatin1(k_last_opened_key),
                  record.last_opened.toString(Qt::ISODateWithMs));
    return result;
}

/**
 * @brief Parses a recent-file record from the existing index schema.
 * @param object Persisted recent-file object.
 * @return Parsed recent-file record.
 */
auto RecentFilesService::from_json(const QJsonObject& object) -> RecentLogFileRecord
{
    RecentLogFileRecord result;
    result.file_path = object.value(QString::fromLatin1(k_file_path_key)).toString();
    result.app_name = object.value(QString::fromLatin1(k_app_name_key)).toString();
    result.last_opened =
        parse_iso_datetime(object.value(QString::fromLatin1(k_last_opened_key)).toString());
    return result;
}

/**
 * @brief Persists recent files while retaining every unrelated root field.
 */
auto RecentFilesService::persist() -> void
{
    if (m_repository != nullptr)
    {
        QJsonObject root = m_repository->load_all();
        QJsonArray files;
        for (const RecentLogFileRecord& record: m_recent_files.items())
        {
            files.append(to_json(record));
        }
        root.insert(QString::fromLatin1(k_recent_files_key), files);
        m_repository->save_all(root);
    }
}
