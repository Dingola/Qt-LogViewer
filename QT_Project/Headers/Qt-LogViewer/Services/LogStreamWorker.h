#pragma once

#include <QMutex>
#include <QObject>
#include <QString>
#include <QVector>
#include <QWaitCondition>
#include <atomic>

#include "Qt-LogViewer/Models/LogEntry.h"
#include "Qt-LogViewer/Services/LogParser.h"

/**
 * @file LogStreamWorker.h
 * @brief Worker object that performs line-by-line parsing in a background thread.
 */

/**
 * @class LogStreamWorker
 * @brief Streams a log file line-by-line, parsing entries and emitting them in batches.
 *
 * Emits:
 *  - entry_batch_parsed()
 *  - progress()
 *  - finished()
 *  - error()
 *
 * Cancellation is cooperative: the current line finishes before exiting.
 */
class LogStreamWorker: public QObject
{
        Q_OBJECT

    public:
        /** Maximum number of emitted batches awaiting durable writer processing. */
        static constexpr qsizetype maximum_in_flight_batches = 4;

        /**
         * @brief Constructs a LogStreamWorker.
         * @param parser Parser instance (copied) used for line parsing.
         * @param parent Optional QObject parent.
         */
        explicit LogStreamWorker(LogParser parser, QObject* parent = nullptr);

    public slots:
        /**
         * @brief Starts reading and parsing the file line-by-line.
         * @param file_path File to read.
         * @param batch_size Number of entries per emitted batch.
         * @param start_offset First source byte to parse.
         * @param end_offset Exclusive source byte boundary, or -1 for the open-time size.
         */
        auto start(const QString& file_path, qsizetype batch_size, qint64 start_offset = 0,
                   qint64 end_offset = -1) -> void;

        /**
         * @brief Requests cancellation of the ongoing operation.
         *
         * Can be called from any thread. Sets an atomic flag that is checked between
         * line reads. Does not forcefully terminate I/O but stops after the current
         * line/token is processed.
         */
        auto cancel() -> void;

        /**
         * @brief Releases capacity after writer processing completes.
         * @param batch_count Number of processed batches to acknowledge.
         *
         * This method is thread-safe and may be called directly from the GUI thread.
         */
        auto acknowledge_batches(qsizetype batch_count) -> void;

    signals:
        /**
         * @brief Emitted when a batch of entries has been parsed.
         * @param file_path The corresponding file path.
         * @param batch Parsed entries.
         */
        auto entry_batch_parsed(const QString& file_path, const QVector<LogEntry>& batch) -> void;

        /**
         * @brief Emitted to report progress.
         * @param file_path File being read.
         * @param bytes_read Bytes read so far.
         * @param total_bytes Total file size.
         */
        auto progress(const QString& file_path, qint64 bytes_read, qint64 total_bytes) -> void;

        /**
         * @brief Emitted when streaming has finished (successfully or cancelled).
         * @param file_path File that finished.
         */
        auto finished(const QString& file_path) -> void;

        /**
         * @brief Emitted when an error occurs while opening or reading.
         * @param file_path File that caused the error.
         * @param message Description of the error.
         */
        auto error(const QString& file_path, const QString& message) -> void;

    private:
        /** @brief Waits for bounded downstream capacity before emitting one batch. */
        auto reserve_batch_slot() -> bool;

        LogParser m_parser;
        std::atomic_bool m_cancelled{false};
        QMutex m_backpressure_mutex;
        QWaitCondition m_backpressure_available;
        qsizetype m_in_flight_batches{0};
};
