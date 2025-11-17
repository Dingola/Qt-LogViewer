/**
 * @file LogLoader.cpp
 * @brief This file contains the implementation of the LogLoader class.
 */

#include "Qt-LogViewer/Services/LogLoader.h"

#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QThread>

#include "Qt-LogViewer/Services/LogStreamWorker.h"

/**
 * @brief Constructs a LogLoader object.
 * @param format_string The log format string for parsing.
 */
LogLoader::LogLoader(const QString& format_string, QObject* parent)
    : QObject(parent), m_parser(format_string), m_worker(nullptr), m_worker_thread(nullptr)
{}

/**
 * @brief Loads and parses a single log file.
 * @param file_path The path to the log file.
 * @return A QVector of LogEntry objects parsed from the file.
 */
auto LogLoader::load_log_file(const QString& file_path) const -> QVector<LogEntry>
{
    QVector<LogEntry> result;
    result = m_parser.parse_file(file_path);
    return result;
}

/**
 * @brief Loads and parses multiple log files, grouping entries by application name.
 * @param file_paths A list of log file paths.
 * @return A map from application name to a vector of LogEntry objects.
 */
auto LogLoader::load_logs_by_app(const QVector<QString>& file_paths) const
    -> QMap<QString, QVector<LogEntry>>
{
    QMap<QString, QVector<LogEntry>> app_logs;

    for (const QString& file_path: file_paths)
    {
        QVector<LogEntry> entries = load_log_file(file_path);
        QString app_name;

        if (!entries.isEmpty())
        {
            app_name = entries.first().get_app_name();
        }
        else
        {
            app_name = identify_app(file_path);
        }

        if (!app_name.isEmpty())
        {
            app_logs[app_name] += entries;
        }
    }

    return app_logs;
}

/**
 * @brief Reads only the first log entry from the given file.
 * @param file_path The path to the log file.
 * @return The first LogEntry if available, otherwise a default LogEntry.
 */
auto LogLoader::read_first_log_entry(const QString& file_path) const -> LogEntry
{
    LogEntry first_entry;
    QFile file(file_path);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);

        while (!in.atEnd() && first_entry.get_app_name().isEmpty())
        {
            const QString line = in.readLine().trimmed();

            if (!line.isEmpty())
            {
                const LogEntry entry = m_parser.parse_line(line, file_path);

                if (!entry.get_app_name().isEmpty())
                {
                    first_entry = entry;
                }
            }
        }
    }

    return first_entry;
}

/**
 * @brief Identifies the application name for a given log file path.
 *        This implementation uses the base file name (without extension) as the app name.
 * @param file_path The path to the log file.
 * @return The application name, or an empty string if not identifiable.
 */
auto LogLoader::identify_app(const QString& file_path) -> QString
{
    QFileInfo info(file_path);
    QString app_name = info.baseName();
    return app_name;
}

/**
 * @brief Starts asynchronous, streaming load of a single log file.
 * @param file_path The path to the log file.
 * @param batch_size The number of entries per emitted batch.
 *
 * Only one streaming operation runs at a time. The next queued file should start
 * on the streaming_idle() signal.
 */
auto LogLoader::load_log_file_async(const QString& file_path, qsizetype batch_size) -> void
{
    if (m_worker_thread == nullptr)
    {
        m_worker_thread = new QThread(this);
        m_worker = new LogStreamWorker(m_parser);
        m_worker->moveToThread(m_worker_thread);

        // Forward worker signals.
        QObject::connect(m_worker, &LogStreamWorker::entry_batch_parsed, this,
                         &LogLoader::entry_batch_parsed, Qt::QueuedConnection);
        QObject::connect(m_worker, &LogStreamWorker::progress, this, &LogLoader::progress,
                         Qt::QueuedConnection);
        QObject::connect(m_worker, &LogStreamWorker::finished, this, &LogLoader::finished,
                         Qt::QueuedConnection);
        QObject::connect(m_worker, &LogStreamWorker::error, this, &LogLoader::error,
                         Qt::QueuedConnection);

        // Quit thread when worker finishes.
        QObject::connect(m_worker, &LogStreamWorker::finished, m_worker_thread, &QThread::quit);

        // Cleanup and emit idle after thread actually stops.
        QObject::connect(m_worker_thread, &QThread::finished, this, [this]() {
            if (m_worker != nullptr)
            {
                m_worker->deleteLater();
                m_worker = nullptr;
            }
            m_worker_thread = nullptr;
            emit streaming_idle();
        });

        // Start work in thread context.
        QObject::connect(
            m_worker_thread, &QThread::started, m_worker,
            [this, file_path, batch_size]() {
                if (m_worker != nullptr)
                {
                    m_worker->start(file_path, batch_size);
                }
            },
            Qt::QueuedConnection);

        m_worker_thread->start();
    }
}

/**
 * @brief Requests cancellation of the current asynchronous load (if any).
 */
auto LogLoader::cancel_async() -> void
{
    if (m_worker_thread != nullptr && m_worker != nullptr)
    {
        m_worker->cancel();
    }
}
