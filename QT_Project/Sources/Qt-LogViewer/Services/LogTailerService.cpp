/**
 * @file LogTailerService.cpp
 * @brief Implements incremental live-log tailing.
 */

#include "Qt-LogViewer/Services/LogTailerService.h"

#include <QFile>
#include <QFileInfo>

/**
 * @brief Constructs the live-tail service.
 * @param log_format Log format used to parse appended lines.
 * @param parent Optional QObject parent.
 */
LogTailerService::LogTailerService(const QString& log_format, QObject* parent)
    : QObject(parent),
      m_watcher(this),
      m_debounce_timer(this),
      m_parser(log_format),
      m_registrations()
{
    m_debounce_timer.setSingleShot(true);
    m_debounce_timer.setInterval(150);

    connect(&m_watcher, &QFileSystemWatcher::fileChanged, this,
            &LogTailerService::handle_file_changed);
    connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this,
            &LogTailerService::handle_directory_changed);
    connect(&m_debounce_timer, &QTimer::timeout, this, &LogTailerService::process_pending_changes);
}

/**
 * @brief Starts tailing a file for one view from the current file end.
 * @param view_id Target view identifier.
 * @param file_path Absolute or relative log-file path.
 */
auto LogTailerService::start_tailing(const QUuid& view_id, const QString& file_path) -> void
{
    const QFileInfo file_info(file_path);
    const QString absolute_file_path = file_info.absoluteFilePath();
    const bool valid_request =
        !view_id.isNull() && file_info.exists() && !has_registration(view_id, absolute_file_path);

    if (valid_request)
    {
        TailRegistration registration;
        registration.view_id = view_id;
        registration.file_path = absolute_file_path;
        registration.offset = file_info.size();
        registration.initial_prefix_fingerprint = get_prefix_fingerprint(absolute_file_path);

        m_registrations.append(registration);
        ensure_watches(absolute_file_path);
    }
}

/**
 * @brief Stops tailing one file for one view.
 * @param view_id Target view identifier.
 * @param file_path Absolute or relative log-file path.
 */
auto LogTailerService::stop_tailing(const QUuid& view_id, const QString& file_path) -> void
{
    QVector<TailRegistration> remaining_registrations;
    const QString absolute_file_path = QFileInfo(file_path).absoluteFilePath();

    for (const TailRegistration& registration: m_registrations)
    {
        const bool remove_registration =
            registration.view_id == view_id && registration.file_path == absolute_file_path;

        if (!remove_registration)
        {
            remaining_registrations.append(registration);
        }
    }

    m_registrations = remaining_registrations;
}

/**
 * @brief Stops every tail registration belonging to a view.
 * @param view_id Target view identifier.
 */
auto LogTailerService::stop_tailing_view(const QUuid& view_id) -> void
{
    QVector<TailRegistration> remaining_registrations;

    for (const TailRegistration& registration: m_registrations)
    {
        if (registration.view_id != view_id)
        {
            remaining_registrations.append(registration);
        }
    }

    m_registrations = remaining_registrations;
}

/**
 * @brief Stops every active tail registration.
 */
auto LogTailerService::stop_all_tailing() -> void
{
    m_debounce_timer.stop();
    m_registrations.clear();
    m_watcher.removePaths(m_watcher.files());
    m_watcher.removePaths(m_watcher.directories());
}

/**
 * @brief Sets the single-shot debounce interval used after watcher notifications.
 * @param debounce_interval_ms Debounce interval in milliseconds; negative values become zero.
 */
auto LogTailerService::set_debounce_interval_ms(int debounce_interval_ms) -> void
{
    const int safe_interval = debounce_interval_ms < 0 ? 0 : debounce_interval_ms;
    m_debounce_timer.setInterval(safe_interval);
}

/**
 * @brief Schedules processing after a watched file changes.
 * @param path Changed file path.
 */
auto LogTailerService::handle_file_changed(const QString& path) -> void
{
    Q_UNUSED(path);
    m_debounce_timer.start();
}

/**
 * @brief Schedules processing after a watched parent directory changes.
 * @param path Changed directory path.
 */
auto LogTailerService::handle_directory_changed(const QString& path) -> void
{
    Q_UNUSED(path);
    m_debounce_timer.start();
}

/**
 * @brief Processes all registrations after the debounce interval expires.
 */
auto LogTailerService::process_pending_changes() -> void
{
    for (TailRegistration& registration: m_registrations)
    {
        ensure_watches(registration.file_path);
        process_registration(registration);
    }
}

/**
 * @brief Ensures the file and its parent directory are watched where possible.
 * @param file_path Absolute file path.
 */
auto LogTailerService::ensure_watches(const QString& file_path) -> void
{
    const QFileInfo file_info(file_path);
    const QString directory_path = file_info.absolutePath();

    if (!directory_path.isEmpty() && !m_watcher.directories().contains(directory_path))
    {
        m_watcher.addPath(directory_path);
    }

    if (file_info.exists() && !m_watcher.files().contains(file_path))
    {
        m_watcher.addPath(file_path);
    }
}

/**
 * @brief Reads, parses, and emits newly complete records for one registration.
 * @param registration Registration state to update.
 */
auto LogTailerService::process_registration(TailRegistration& registration) -> void
{
    const QFileInfo file_info(registration.file_path);

    if (!file_info.exists())
    {
        return;
    }

    const QByteArray current_prefix_fingerprint = get_prefix_fingerprint(registration.file_path);

    const bool file_truncated = file_info.size() < registration.offset;

    // Compare only the number of prefix bytes that existed during the
    // previous check. Normal appends extend the returned prefix for small
    // files, but must not be interpreted as replacement.
    const qsizetype previous_prefix_size = registration.initial_prefix_fingerprint.size();

    const bool file_replaced =
        previous_prefix_size > 0 && current_prefix_fingerprint.left(previous_prefix_size) !=
                                        registration.initial_prefix_fingerprint;

    if (file_truncated || file_replaced)
    {
        registration.offset = 0;
        registration.incomplete_line.clear();
    }

    QFile file(registration.file_path);

    if (!file.open(QIODevice::ReadOnly))
    {
        return;
    }

    if (!file.seek(registration.offset))
    {
        file.close();
        return;
    }

    QByteArray data = registration.incomplete_line;
    data.append(file.readAll());
    registration.offset = file.pos();

    const bool has_incomplete_line = !data.endsWith('\n');
    QList<QByteArray> lines = data.split('\n');

    registration.incomplete_line.clear();

    if (has_incomplete_line && !lines.isEmpty())
    {
        registration.incomplete_line = lines.takeLast();
    }

    QVector<LogEntry> entries;

    for (const QByteArray& line: lines)
    {
        if (line.isEmpty())
        {
            continue;
        }

        const LogEntry entry =
            m_parser.parse_line(QString::fromUtf8(line).trimmed(), registration.file_path);

        if (!entry.get_level().isEmpty())
        {
            entries.append(entry);
        }
    }

    registration.initial_prefix_fingerprint = current_prefix_fingerprint;

    if (!entries.isEmpty())
    {
        emit entries_available(registration.view_id, registration.file_path, entries);
    }

    file.close();
}

/**
 * @brief Reads a stable prefix used to distinguish replacement from append.
 * @param file_path Absolute file path.
 * @return The first up to 4096 bytes, or an empty array if the file cannot be read.
 */
auto LogTailerService::get_prefix_fingerprint(const QString& file_path) -> QByteArray
{
    QByteArray fingerprint;
    QFile file(file_path);

    if (file.open(QIODevice::ReadOnly))
    {
        fingerprint = file.read(4096);
        file.close();
    }

    return fingerprint;
}

/**
 * @brief Determines whether a registration already exists.
 * @param view_id Target view identifier.
 * @param file_path Absolute file path.
 * @return True if the file is already tailed for the view.
 */
auto LogTailerService::has_registration(const QUuid& view_id,
                                        const QString& file_path) const -> bool
{
    bool found = false;

    for (const TailRegistration& registration: m_registrations)
    {
        if (registration.view_id == view_id && registration.file_path == file_path)
        {
            found = true;
        }
    }

    return found;
}
