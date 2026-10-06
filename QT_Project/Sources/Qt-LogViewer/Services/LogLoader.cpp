/**
 * @file LogLoader.cpp
 * @brief This file contains the implementation of the LogLoader class.
 */

#include "Qt-LogViewer/Services/LogLoader.h"

#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QThread>
#include <limits>

#include "Qt-LogViewer/Services/LogStreamWorker.h"

/**
 * @brief Constructs a LogLoader object.
 * @param profile Parsing profile used for all loaded records.
 * @param parent Optional QObject parent.
 *
 * All records use the supplied parsing
 * profile. QObject ownership follows the optional parent
 * supplied by the caller.
 */
LogLoader::LogLoader(const LogParsingProfile& profile, QObject* parent)
    : QObject(parent), m_parser(profile), m_worker(nullptr), m_worker_thread(nullptr)
{}

/**
 * @brief Loads and parses a single log file.
 * @param file_path The path to the log file.
 * @return A QVector of LogEntry objects parsed from the file.
 */
auto LogLoader::load_log_file(const QString& file_path) const -> QVector<LogEntry>
{
    return load_log_file(file_path, m_parser.get_profile());
}

/**
 * @brief Loads and parses a single log file with an explicitly selected profile.
 * @param file_path Path of the log file.
 * @param profile Parsing profile selected for this import.
 * @return Parsed log entries.
 */
auto LogLoader::load_log_file(const QString& file_path,
                              const LogParsingProfile& profile) const -> QVector<LogEntry>
{
    const LogParser parser(profile);
    return parser.parse_file(file_path);
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
                const LogParseOutcome outcome = m_parser.parse_line(line, file_path);

                if (outcome.succeeded())
                {
                    first_entry = outcome.entry.value();
                }
            }
        }
    }

    return first_entry;
}

/**
 * @brief Parses the first records of a file without starting an import.
 * @param file_path Path of the file to preview.
 * @param profile Parsing profile to evaluate.
 * @param maximum_record_count Maximum number of non-empty records to return.
 * @return Parse outcomes in source order, including structured failures.
 */
auto LogLoader::preview_log_file(const QString& file_path, const LogParsingProfile& profile,
                                 qsizetype maximum_record_count) const -> QVector<LogParseOutcome>
{
    QVector<LogParseOutcome> outcomes;
    QFile file(file_path);

    const bool can_preview =
        maximum_record_count > 0 && file.open(QIODevice::ReadOnly | QIODevice::Text);

    if (can_preview)
    {
        const LogParser parser(profile);
        QTextStream input(&file);
        qsizetype line_number = 0;

        while (!input.atEnd() && outcomes.size() < maximum_record_count &&
               line_number < std::numeric_limits<qsizetype>::max())
        {
            const QString record = input.readLine();
            ++line_number;

            if (!record.trimmed().isEmpty())
            {
                outcomes.append(parser.parse_line(record, file_path, line_number));
            }
        }
    }

    return outcomes;
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
    load_log_file_async(file_path, batch_size, m_parser.get_profile(), 0, -1);
}

/**
 * @brief Starts asynchronous loading with an explicitly selected parsing profile.
 * @param file_path Path of the log file.
 * @param batch_size Number of entries per emitted batch.
 * @param profile Parsing profile selected for this import.
 * @param start_offset First source byte to parse.
 * @param end_offset Exclusive source byte boundary, or -1 for the open-time size.
 */
auto LogLoader::load_log_file_async(const QString& file_path, qsizetype batch_size,
                                    const LogParsingProfile& profile, qint64 start_offset,
                                    qint64 end_offset) -> void
{
    if (m_worker_thread == nullptr)
    {
        m_worker_thread = new QThread(this);
        m_worker = new LogStreamWorker(LogParser(profile));
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
            [this, file_path, batch_size, start_offset, end_offset]() {
                if (m_worker != nullptr)
                {
                    m_worker->start(file_path, batch_size, start_offset, end_offset);
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

/**
 * @brief Releases parser capacity after downstream writer processing.
 * @param batch_count Number of stored batches to acknowledge.
 */
auto LogLoader::acknowledge_batches(qsizetype batch_count) -> void
{
    if (m_worker != nullptr)
    {
        m_worker->acknowledge_batches(batch_count);
    }
}
