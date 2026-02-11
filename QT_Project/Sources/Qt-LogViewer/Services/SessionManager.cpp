#include "Qt-LogViewer/Services/SessionManager.h"

#include <QDateTime>
#include <QFileInfo>
#include <QJsonArray>

// Concrete include for forward-declared repository type
#include "Qt-LogViewer/Services/SessionRepository.h"

namespace
{
// Root JSON keys (stable, not localized).
constexpr auto k_schema_version_key = "schema_version";
constexpr auto k_recent_files_key = "recent_files";
constexpr auto k_recent_sessions_key = "recent_sessions";
constexpr auto k_last_session_id_key = "last_session_id";

// Recent files fields.
constexpr auto k_file_path_key = "file_path";
constexpr auto k_app_name_key = "app_name";
constexpr auto k_last_opened_key = "last_opened";

// Recent sessions fields.
constexpr auto k_session_id_key = "id";
constexpr auto k_session_name_key = "name";
constexpr auto k_created_at_key = "created_at";
constexpr auto k_session_last_opened_key = "last_opened";

/**
 * @brief Parses an ISO datetime string to QDateTime (UTC if 'Z' present).
 * @param s String in ISO 8601 format.
 * @return QDateTime parsed; invalid if empty or malformed.
 */
auto parse_iso_datetime(const QString& s) -> QDateTime
{
    QDateTime dt = QDateTime::fromString(s, Qt::ISODateWithMs);
    if (!dt.isValid())
    {
        dt = QDateTime::fromString(s, Qt::ISODate);
    }
    return dt;
}

/**
 * @brief Formats QDateTime to ISO 8601 string with milliseconds.
 * @param dt The datetime to format.
 * @return ISO 8601 string.
 */
auto to_iso_datetime(const QDateTime& dt) -> QString
{
    QString s = dt.toString(Qt::ISODateWithMs);
    return s;
}
}  // namespace

/**
 * @brief Constructs a SessionManager with a given repository and optional parent.
 * @param repository Persistence repository (owned by this manager).
 * @param parent Optional QObject parent.
 */
SessionManager::SessionManager(SessionRepository* repository, QObject* parent)
    : QObject(parent),
      m_repository(repository),
      m_recent_files(),
      m_recent_sessions(),
      m_last_session_id(),
      m_current_session_id()
{}

/**
 * @brief Loads state from repository into memory (recent files and sessions).
 *
 * Safe to call multiple times; replaces the in-memory lists.
 */
auto SessionManager::initialize_from_storage() -> void
{
    const QJsonObject root = load_root();

    m_recent_files.clear();
    m_recent_sessions.clear();
    m_last_session_id.clear();
    m_current_session_id.clear();

    if (root.contains(QString::fromLatin1(k_recent_files_key)))
    {
        const QJsonArray arr = root.value(QString::fromLatin1(k_recent_files_key)).toArray();
        for (const auto& v: arr)
        {
            const QJsonObject obj = v.toObject();
            m_recent_files.append(from_json_recent_file(obj));
        }
    }

    if (root.contains(QString::fromLatin1(k_recent_sessions_key)))
    {
        const QJsonArray arr = root.value(QString::fromLatin1(k_recent_sessions_key)).toArray();
        for (const auto& v: arr)
        {
            const QJsonObject obj = v.toObject();
            m_recent_sessions.append(from_json_recent_session(obj));
        }
    }

    if (root.contains(QString::fromLatin1(k_last_session_id_key)))
    {
        m_last_session_id = root.value(QString::fromLatin1(k_last_session_id_key)).toString();
    }

    sort_recent_files_mru();
    sort_recent_sessions_mru();

    emit recent_log_files_changed(m_recent_files);
    emit recent_sessions_changed(m_recent_sessions);
}

/**
 * @brief Returns the list of recent log files.
 * @return Vector of RecentLogFileRecord.
 */
auto SessionManager::get_recent_log_files() const -> QVector<RecentLogFileRecord>
{
    QVector<RecentLogFileRecord> items = m_recent_files;
    return items;
}

/**
 * @brief Adds/updates a recent log file record and persists the root document.
 * @param file_info LogFileInfo from which file_path and app_name are derived.
 *
 * Behavior:
 * - Deduplicate by file_path (case-sensitive as stored).
 * - Update last_opened to current time.
 * - Move updated item to front (most recent first).
 */
auto SessionManager::add_recent_log_file(const LogFileInfo& file_info) -> void
{
    RecentLogFileRecord rec{file_info.get_file_path(), file_info.get_app_name(),
                            QDateTime::currentDateTime()};
    upsert_recent_file(rec);
    sort_recent_files_mru();

    const QJsonObject root = make_root_from_current();
    persist_root(root);

    emit recent_log_files_changed(m_recent_files);
}

/**
 * @brief Clears the recent log files list and persists the change.
 */
auto SessionManager::clear_recent_log_files() -> void
{
    m_recent_files.clear();

    const QJsonObject root = make_root_from_current();
    persist_root(root);

    emit recent_log_files_changed(m_recent_files);
}

/**
 * @brief Returns the list of recent sessions metadata.
 * @return Vector of RecentSessionRecord.
 */
auto SessionManager::get_recent_sessions() const -> QVector<RecentSessionRecord>
{
    QVector<RecentSessionRecord> items = m_recent_sessions;
    return items;
}

/**
 * @brief Saves or updates a session metadata entry and persists the root document.
 * @param session_id Unique identifier of the session.
 * @param name Human-readable session name.
 * @param is_open_update If true, updates last_opened; if false, ensures created_at exists.
 *
 * Behavior:
 * - If the session id exists, update its name and timestamps accordingly.
 * - If not, create a new record with timestamps.
 * - Most recent sessions appear first.
 */
auto SessionManager::upsert_session_metadata(const QString& session_id, const QString& name,
                                             bool is_open_update) -> void
{
    const QDateTime now = QDateTime::currentDateTime();
    bool found = false;

    for (auto& s: m_recent_sessions)
    {
        if (s.id == session_id)
        {
            s.name = name;
            if (is_open_update)
            {
                s.last_opened = now;
            }
            if (!s.created_at.isValid())
            {
                s.created_at = now;
            }
            found = true;
        }
    }

    if (!found)
    {
        RecentSessionRecord rec{session_id, name, now, is_open_update ? now : QDateTime()};
        upsert_recent_session(rec);
    }

    sort_recent_sessions_mru();

    const QJsonObject root = make_root_from_current();
    persist_root(root);

    emit recent_sessions_changed(m_recent_sessions);
}

/**
 * @brief Deletes a session JSON file and removes its metadata entry, then persists root.
 * @param session_id The session identifier.
 * @return True if file existed and metadata entry was removed; false otherwise.
 */
auto SessionManager::delete_session(const QString& session_id) -> bool
{
    const bool removed_meta = remove_recent_session_by_id(session_id);
    const bool removed_file = m_repository != nullptr && m_repository->delete_session(session_id);

    if (removed_meta)
    {
        const QJsonObject root = make_root_from_current();
        persist_root(root);
        emit recent_sessions_changed(m_recent_sessions);
    }

    const bool success = (removed_meta || removed_file);
    return success;
}

/**
 * @brief Loads a full session JSON object from repository.
 * @param session_id The session identifier.
 * @return QJsonObject representing the session, or empty object if not found.
 */
auto SessionManager::load_session(const QString& session_id) const -> QJsonObject
{
    QJsonObject obj;

    if (m_repository != nullptr)
    {
        obj = m_repository->load_session(session_id);
    }

    return obj;
}

/**
 * @brief Saves a full session JSON object under its id (atomic write).
 * @param session_id The session identifier.
 * @param session_obj The JSON object to persist.
 */
auto SessionManager::save_session(const QString& session_id, const QJsonObject& session_obj) -> void
{
    if (m_repository != nullptr)
    {
        m_repository->save_session(session_id, session_obj);
    }
}

/**
 * @brief Returns the current session id in use (in-memory only).
 * @return Current session id or empty string if not set.
 */
auto SessionManager::get_current_session_id() const -> QString
{
    return m_current_session_id;
}

/**
 * @brief Sets the current session id in use (in-memory).
 * @param session_id The session identifier to set as current.
 */
auto SessionManager::set_current_session_id(const QString& session_id) -> void
{
    m_current_session_id = session_id;
}

/**
 * @brief Checks if there is a current session id set (in-memory).
 * @return True if a current session id is set; false otherwise.
 */
auto SessionManager::has_current_session() const -> bool
{
    return !m_current_session_id.isEmpty();
}

/**
 * @brief Returns the last session id stored in the root document (optional).
 * @return Last session id or empty string if not set.
 */
auto SessionManager::get_last_session_id() const -> QString
{
    QString id = m_last_session_id;
    return id;
}

/**
 * @brief Sets the last session id in the root document and persists the change.
 * @param session_id The session identifier to store.
 */
auto SessionManager::set_last_session_id(const QString& session_id) -> void
{
    m_last_session_id = session_id;

    const QJsonObject root = make_root_from_current();
    persist_root(root);
}

/**
 * @brief Converts a RecentLogFileRecord to JSON.
 * @param rec The record to convert.
 * @return QJsonObject representation.
 */
auto SessionManager::to_json(const RecentLogFileRecord& rec) -> QJsonObject
{
    QJsonObject obj;
    obj.insert(QString::fromLatin1(k_file_path_key), rec.file_path);
    obj.insert(QString::fromLatin1(k_app_name_key), rec.app_name);
    obj.insert(QString::fromLatin1(k_last_opened_key),
               rec.last_opened.isValid() ? to_iso_datetime(rec.last_opened) : QString());
    return obj;
}

/**
 * @brief Converts a RecentSessionRecord to JSON.
 * @param rec The record to convert.
 * @return QJsonObject representation.
 */
auto SessionManager::to_json(const RecentSessionRecord& rec) -> QJsonObject
{
    QJsonObject obj;
    obj.insert(QString::fromLatin1(k_session_id_key), rec.id);
    obj.insert(QString::fromLatin1(k_session_name_key), rec.name);
    obj.insert(QString::fromLatin1(k_created_at_key),
               rec.created_at.isValid() ? to_iso_datetime(rec.created_at) : QString());
    obj.insert(QString::fromLatin1(k_session_last_opened_key),
               rec.last_opened.isValid() ? to_iso_datetime(rec.last_opened) : QString());
    return obj;
}

/**
 * @brief Parses a recent file record from JSON.
 * @param obj The JSON object.
 * @return RecentLogFileRecord value.
 */
auto SessionManager::from_json_recent_file(const QJsonObject& obj) -> RecentLogFileRecord
{
    RecentLogFileRecord rec;
    rec.file_path = obj.value(QString::fromLatin1(k_file_path_key)).toString();
    rec.app_name = obj.value(QString::fromLatin1(k_app_name_key)).toString();
    rec.last_opened =
        parse_iso_datetime(obj.value(QString::fromLatin1(k_last_opened_key)).toString());
    return rec;
}

/**
 * @brief Parses a recent session record from JSON.
 * @param obj The JSON object.
 * @return RecentSessionRecord value.
 */
auto SessionManager::from_json_recent_session(const QJsonObject& obj) -> RecentSessionRecord
{
    RecentSessionRecord rec;
    rec.id = obj.value(QString::fromLatin1(k_session_id_key)).toString();
    rec.name = obj.value(QString::fromLatin1(k_session_name_key)).toString();
    rec.created_at =
        parse_iso_datetime(obj.value(QString::fromLatin1(k_created_at_key)).toString());
    rec.last_opened =
        parse_iso_datetime(obj.value(QString::fromLatin1(k_session_last_opened_key)).toString());
    return rec;
}

/**
 * @brief Builds the root JSON document from current in-memory state.
 * @return QJsonObject root.
 */
auto SessionManager::make_root_from_current() const -> QJsonObject
{
    QJsonObject root;

    // Inject schema_version (kept by repository on save, but provide here for clarity).
    root.insert(QString::fromLatin1(k_schema_version_key), SessionRepository::get_schema_version());

    // Recent files
    QJsonArray files;
    for (const auto& rf: m_recent_files)
    {
        files.append(to_json(rf));
    }
    root.insert(QString::fromLatin1(k_recent_files_key), files);

    // Recent sessions
    QJsonArray sessions;
    for (const auto& rs: m_recent_sessions)
    {
        sessions.append(to_json(rs));
    }
    root.insert(QString::fromLatin1(k_recent_sessions_key), sessions);

    // Last session id
    if (!m_last_session_id.isEmpty())
    {
        root.insert(QString::fromLatin1(k_last_session_id_key), m_last_session_id);
    }

    return root;
}

/**
 * @brief Persists the root JSON document using repository (atomic write).
 * @param root Root object to save.
 */
auto SessionManager::persist_root(const QJsonObject& root) -> void
{
    if (m_repository != nullptr)
    {
        m_repository->save_all(root);
    }
}

/**
 * @brief Loads the root JSON document using repository; ensures defaults.
 * @return Root JSON object.
 */
auto SessionManager::load_root() const -> QJsonObject
{
    QJsonObject root;

    if (m_repository != nullptr)
    {
        root = m_repository->load_all();
    }

    return root;
}

/**
 * @brief Sort recent files by last_opened descending, then by file_name ascending as tiebreaker.
 */
auto SessionManager::sort_recent_files_mru() -> void
{
    std::sort(m_recent_files.begin(), m_recent_files.end(),
              [](const RecentLogFileRecord& a, const RecentLogFileRecord& b) {
                  if (a.last_opened == b.last_opened)
                  {
                      const QString af = QFileInfo(a.file_path).fileName();
                      const QString bf = QFileInfo(b.file_path).fileName();
                      return af.localeAwareCompare(bf) < 0;
                  }
                  return a.last_opened > b.last_opened;
              });
}

/**
 * @brief Sort recent sessions by last_opened or created_at (most recent first).
 */
auto SessionManager::sort_recent_sessions_mru() -> void
{
    std::sort(m_recent_sessions.begin(), m_recent_sessions.end(),
              [](const RecentSessionRecord& a, const RecentSessionRecord& b) {
                  const QDateTime ad = a.last_opened.isValid() ? a.last_opened : a.created_at;
                  const QDateTime bd = b.last_opened.isValid() ? b.last_opened : b.created_at;
                  if (ad == bd)
                  {
                      return a.name.localeAwareCompare(b.name) < 0;
                  }
                  return ad > bd;
              });
}

/**
 * @brief Removes a recent session by id.
 * @param session_id The id to remove.
 * @return True if removed.
 */
auto SessionManager::remove_recent_session_by_id(const QString& session_id) -> bool
{
    bool removed = false;

    QVector<RecentSessionRecord> keep;
    keep.reserve(m_recent_sessions.size());

    for (const auto& s: m_recent_sessions)
    {
        if (s.id == session_id)
        {
            removed = true;
        }
        else
        {
            keep.append(s);
        }
    }

    m_recent_sessions = keep;
    return removed;
}

/**
 * @brief Upserts a recent session record into the list.
 * @param rec The record to upsert.
 */
auto SessionManager::upsert_recent_session(const RecentSessionRecord& rec) -> void
{
    bool exists = false;

    for (auto& s: m_recent_sessions)
    {
        if (s.id == rec.id)
        {
            s = rec;
            exists = true;
        }
    }

    if (!exists)
    {
        m_recent_sessions.append(rec);
    }
}

/**
 * @brief Upserts a recent file record into the list (dedupe by file_path).
 * @param rec The record to upsert.
 */
auto SessionManager::upsert_recent_file(const RecentLogFileRecord& rec) -> void
{
    bool exists = false;

    for (auto& f: m_recent_files)
    {
        if (f.file_path == rec.file_path)
        {
            f = rec;
            exists = true;
        }
    }

    if (!exists)
    {
        m_recent_files.append(rec);
    }
}
