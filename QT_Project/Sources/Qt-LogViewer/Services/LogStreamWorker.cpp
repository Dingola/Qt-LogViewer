/**
 * @file LogStreamWorker.cpp
 * @brief Implementation of LogStreamWorker.
 */

#include "Qt-LogViewer/Services/LogStreamWorker.h"

#include <QFile>
#include <QMutexLocker>
#include <algorithm>

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

        qsizetype line_number = 0;

        while (!file.atEnd() && !m_cancelled.load())
        {
            const qint64 byte_offset = file.pos();
            QByteArray record_bytes = file.readLine();
            if (record_bytes.endsWith('\n'))
            {
                record_bytes.chop(1);
            }
            if (record_bytes.endsWith('\r'))
            {
                record_bytes.chop(1);
            }

            ++line_number;
            const QString line = QString::fromUtf8(record_bytes);
            const LogParseOutcome outcome = m_parser.parse_line(line, file_path, line_number);

            if (outcome.succeeded())
            {
                LogEntry entry = outcome.entry.value();
                entry.set_source_range(byte_offset, record_bytes.size());
                batch.append(std::move(entry));
            }

            if (batch.size() >= batch_size && reserve_batch_slot())
            {
                emit entry_batch_parsed(file_path, batch);
                batch.clear();
            }

            const qint64 pos = file.pos();
            if (pos - last_progress >= 1024 * 1024 || (file.atEnd() && pos != last_progress))
            {
                emit progress(file_path, pos, total);
                last_progress = pos;
            }
        }

        if (!batch.isEmpty() && !m_cancelled.load() && reserve_batch_slot())
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
    m_cancelled.store(true, std::memory_order_release);
    QMutexLocker locker(&m_backpressure_mutex);
    m_backpressure_available.wakeAll();
}

/**
 * @brief Releases capacity after writer processing completes.
 * @param batch_count Number of processed batches to acknowledge.
 */
auto LogStreamWorker::acknowledge_batches(qsizetype batch_count) -> void
{
    if (batch_count > 0)
    {
        QMutexLocker locker(&m_backpressure_mutex);
        m_in_flight_batches = std::max<qsizetype>(0, m_in_flight_batches - batch_count);
        m_backpressure_available.wakeAll();
    }
}

/** @brief Waits until another batch may enter the downstream pipeline. */
auto LogStreamWorker::reserve_batch_slot() -> bool
{
    QMutexLocker locker(&m_backpressure_mutex);

    while (!m_cancelled.load(std::memory_order_acquire) &&
           m_in_flight_batches >= maximum_in_flight_batches)
    {
        m_backpressure_available.wait(&m_backpressure_mutex);
    }

    const bool reserved = !m_cancelled.load(std::memory_order_acquire);
    if (reserved)
    {
        ++m_in_flight_batches;
    }

    return reserved;
}
