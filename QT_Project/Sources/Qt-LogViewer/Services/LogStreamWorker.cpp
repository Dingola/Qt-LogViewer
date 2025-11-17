/**
 * @file LogStreamWorker.cpp
 * @brief Implementation of LogStreamWorker.
 */

#include "Qt-LogViewer/Services/LogStreamWorker.h"

#include <QFile>
#include <QTextStream>

/**
 * @brief Constructs a LogStreamWorker.
 * @param parser Parser instance (copied) used for line parsing.
 * @param parent Optional QObject parent.
 */
LogStreamWorker::LogStreamWorker(LogParser parser, QObject* parent)
    : QObject(parent), m_parser(std::move(parser))
{
    m_cancelled.store(false);
}

/**
 * @brief Starts reading and parsing the file line-by-line.
 * @param file_path File to read.
 * @param batch_size Number of entries per emitted batch.
 */
auto LogStreamWorker::start(const QString& file_path, qsizetype batch_size) -> void
{
    QVector<LogEntry> batch;
    QFile file(file_path);
    qint64 last_progress = 0;
    qint64 total = 0;

    if (file.exists())
    {
        total = file.size();
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        emit error(file_path, QStringLiteral("Failed to open file for reading."));
        emit finished(file_path);
    }
    else
    {
        emit progress(file_path, 0, total);

        QTextStream in(&file);

        while (!in.atEnd() && !m_cancelled.load())
        {
            const QString line = in.readLine();
            const LogEntry entry = m_parser.parse_line(line, file_path);

            if (!entry.get_level().isEmpty())
            {
                batch.append(entry);
            }

            if (batch.size() >= batch_size)
            {
                emit entry_batch_parsed(file_path, batch);
                batch.clear();
            }

            const qint64 pos = file.pos();
            if (pos - last_progress >= 1024 * 1024 || (in.atEnd() && pos != last_progress))
            {
                emit progress(file_path, pos, total);
                last_progress = pos;
            }
        }

        if (!batch.isEmpty())
        {
            emit entry_batch_parsed(file_path, batch);
        }

        emit finished(file_path);
    }
}

/**
 * @brief Requests cancellation of the ongoing operation.
 */
auto LogStreamWorker::cancel() -> void
{
    m_cancelled.store(true);
}
