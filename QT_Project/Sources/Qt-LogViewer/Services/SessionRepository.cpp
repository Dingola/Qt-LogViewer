#include "Qt-LogViewer/Services/SessionRepository.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSaveFile>
#include <QStandardPaths>

namespace
{
// Schema version constant.
constexpr int k_schema_version_value = 1;

// File and directory names.
constexpr auto k_root_filename = "sessions.json";
constexpr auto k_sessions_dirname = "sessions";

// JSON keys (stable, not localized).
constexpr auto k_schema_version_key = "schema_version";
constexpr auto k_recent_files_key = "recent_files";
constexpr auto k_recent_sessions_key = "recent_sessions";

/**
 * @brief Ensures a directory exists, creating it if necessary.
 * @param dir_path The directory path to ensure.
 */
auto ensure_dir(const QString& dir_path) -> void
{
    QDir dir(dir_path);
    if (!dir.exists())
    {
        dir.mkpath(QStringLiteral("."));
    }
}
}  // namespace

/**
 * @brief Constructs a repository rooted at the application configuration location.
 * @param app_config_subdir Optional subdirectory name under AppConfigLocation,
 *        defaults to "Qt-LogViewer".
 * @param parent Optional QObject parent.
 */
SessionRepository::SessionRepository(QString app_config_subdir, QObject* parent)
    : QObject(parent), m_app_config_subdir(app_config_subdir)
{}

/**
 * @brief Returns the schema version handled by this repository.
 * @return Integer schema version.
 */
auto SessionRepository::get_schema_version() -> int
{
    int version = k_schema_version_value;
    return version;
}

/**
 * @brief Returns the base writable app configuration directory.
 * @return Absolute base directory path, potentially including the configured subdir.
 */
auto SessionRepository::get_app_config_root() const -> QString
{
    const QString base = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QString path = base;

    if (!m_app_config_subdir.isEmpty())
    {
        QDir dir(base);
        path = dir.filePath(m_app_config_subdir);
    }

    return path;
}

/**
 * @brief Returns the absolute path of the root JSON file.
 * @return Absolute file path to the root document.
 */
auto SessionRepository::get_root_file_path() const -> QString
{
    const QString root_dir = get_app_config_root();
    QDir dir(root_dir);
    const QString path = dir.filePath(QString::fromLatin1(k_root_filename));
    return path;
}

/**
 * @brief Loads the entire root JSON document.
 * @return QJsonObject containing the parsed JSON. If missing or invalid, returns
 *         an object with defaults for known keys.
 *
 * Defaults ensured:
 * - k_schema_version_key (int)
 * - k_recent_files_key (array)
 * - k_recent_sessions_key (array)
 */
auto SessionRepository::load_all() const -> QJsonObject
{
    const QString path = get_root_file_path();
    QJsonObject result;

    QFile file(path);
    if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        const QByteArray content = file.readAll();
        file.close();

        const QJsonDocument doc = QJsonDocument::fromJson(content);
        if (doc.isObject())
        {
            result = doc.object();
        }
    }

    if (!result.contains(QString::fromLatin1(k_schema_version_key)))
    {
        result.insert(QString::fromLatin1(k_schema_version_key), get_schema_version());
    }
    if (!result.contains(QString::fromLatin1(k_recent_files_key)))
    {
        result.insert(QString::fromLatin1(k_recent_files_key), QJsonArray());
    }
    if (!result.contains(QString::fromLatin1(k_recent_sessions_key)))
    {
        result.insert(QString::fromLatin1(k_recent_sessions_key), QJsonArray());
    }

    return result;
}

/**
 * @brief Atomically saves the entire root JSON document.
 * @param root The JSON object to serialize and write.
 *
 * Ensures the target directory exists and uses QSaveFile to avoid partial writes.
 * Automatically injects the schema version if missing.
 */
auto SessionRepository::save_all(const QJsonObject& root) const -> void
{
    const QString root_dir = get_app_config_root();
    ensure_dir(root_dir);

    const QString path = get_root_file_path();
    QSaveFile save_file(path);

    if (save_file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QJsonObject obj = root;
        if (!obj.contains(QString::fromLatin1(k_schema_version_key)))
        {
            obj.insert(QString::fromLatin1(k_schema_version_key), get_schema_version());
        }

        const QJsonDocument doc(obj);
        const QByteArray bytes = doc.toJson(QJsonDocument::Indented);

        save_file.write(bytes);
        save_file.commit();
    }
}

/**
 * @brief Returns the absolute directory path used for per-session files.
 * @return Absolute directory path.
 */
auto SessionRepository::get_sessions_dir_path() const -> QString
{
    const QString root_dir = get_app_config_root();
    QDir dir(root_dir);
    const QString path = dir.filePath(QString::fromLatin1(k_sessions_dirname));
    return path;
}

/**
 * @brief Computes the absolute file path for a specific session id.
 * @param session_id The session identifier.
 * @return Absolute file path within the sessions directory.
 */
auto SessionRepository::get_session_file_path(const QString& session_id) const -> QString
{
    const QString dir_path = get_sessions_dir_path();
    QDir dir(dir_path);
    const QString filename = session_id + QStringLiteral(".json");
    const QString path = dir.filePath(filename);
    return path;
}

/**
 * @brief Loads a single session JSON document by id.
 * @param session_id The session identifier.
 * @return Parsed session JSON object, or an empty object if the file does not exist or is invalid.
 */
auto SessionRepository::load_session(const QString& session_id) const -> QJsonObject
{
    const QString path = get_session_file_path(session_id);
    QJsonObject result;

    QFile file(path);
    if (file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        const QByteArray content = file.readAll();
        file.close();

        const QJsonDocument doc = QJsonDocument::fromJson(content);
        if (doc.isObject())
        {
            result = doc.object();
        }
    }

    return result;
}

/**
 * @brief Atomically saves a single session JSON document under its id.
 * @param session_id The session identifier.
 * @param obj The session JSON object to write.
 *
 * Ensures the sessions directory exists and uses QSaveFile to avoid partial writes.
 * Injects schema version if missing.
 */
auto SessionRepository::save_session(const QString& session_id,
                                     const QJsonObject& obj) const -> void
{
    const QString dir_path = get_sessions_dir_path();
    ensure_dir(dir_path);

    const QString path = get_session_file_path(session_id);
    QSaveFile save_file(path);

    if (save_file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QJsonObject session_obj = obj;
        if (!session_obj.contains(QString::fromLatin1(k_schema_version_key)))
        {
            session_obj.insert(QString::fromLatin1(k_schema_version_key), get_schema_version());
        }

        const QJsonDocument doc(session_obj);
        const QByteArray bytes = doc.toJson(QJsonDocument::Indented);

        save_file.write(bytes);
        save_file.commit();
    }
}

/**
 * @brief Deletes a stored session JSON file if present.
 * @param session_id The session identifier.
 * @return True if a file existed and was deleted; false otherwise.
 */
auto SessionRepository::delete_session(const QString& session_id) const -> bool
{
    const QString path = get_session_file_path(session_id);
    bool deleted = false;

    QFile file(path);
    if (file.exists())
    {
        deleted = file.remove();
    }

    return deleted;
}
