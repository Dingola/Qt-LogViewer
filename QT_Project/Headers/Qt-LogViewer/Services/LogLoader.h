#pragma once

#include <QMap>
#include <QObject>
#include <QString>
#include <QVector>

#include "Qt-LogViewer/Models/LogEntry.h"
#include "Qt-LogViewer/Services/LogParser.h"

/**
 * @file LogLoader.h
 * @brief This file contains the definition of the LogLoader class.
 */

class LogStreamWorker;

/**
 * @class LogLoader
 * @brief Loads log files, identifies their application, and parses them into LogEntry objects.
 *
 * This class is responsible for loading log files from disk, determining the application
 * source for each log, and using LogParser to extract LogEntry objects. It provides both
 * eager (full-file) and streaming (line-by-line) loading modes.
 */
class LogLoader: public QObject
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs a LogLoader object.
         * @param format_string The log format string for parsing.
         * @param parent Optional QObject parent.
         */
        explicit LogLoader(const QString& format_string, QObject* parent = nullptr);

        /**
         * @brief Destroys the LogLoader object.
         */
        ~LogLoader() override = default;

        /**
         * @brief Loads and parses a single log file.
         * @param file_path The path to the log file.
         * @return A QVector of LogEntry objects parsed from the file.
         */
        [[nodiscard]] auto load_log_file(const QString& file_path) const -> QVector<LogEntry>;

        /**
         * @brief Loads and parses multiple log files.
         * @param file_paths A list of log file paths.
         * @return A map from application name to a vector of LogEntry objects.
         */
        [[nodiscard]] auto load_logs_by_app(const QVector<QString>& file_paths) const
            -> QMap<QString, QVector<LogEntry>>;

        /**
         * @brief Reads only the first log entry from the given file.
         * @param file_path The path to the log file.
         * @return The first LogEntry if available, otherwise a default LogEntry.
         */
        [[nodiscard]] auto read_first_log_entry(const QString& file_path) const -> LogEntry;

        /**
         * @brief Identifies the application name for a given log file path.
         * @param file_path The path to the log file.
         * @return The application name, or an empty string if not identifiable.
         */
        [[nodiscard]] static auto identify_app(const QString& file_path) -> QString;

        /**
         * @brief Starts asynchronous, streaming load of a single log file.
         *        Emits batches of parsed entries and progress until finished or cancelled.
         * @param file_path The path to the log file.
         * @param batch_size The number of entries per emitted batch (default 1000).
         */
        auto load_log_file_async(const QString& file_path, qsizetype batch_size = 1000) -> void;

        /**
         * @brief Requests cancellation of the current asynchronous load (if any).
         *
         * This method calls the worker's cancel() directly (atomic flag set) instead of
         * posting a queued meta-call, ensuring cancellation becomes effective immediately
         * inside the streaming loop.
         */
        auto cancel_async() -> void;

    signals:
        /**
         * @brief Emitted when a batch of entries has been parsed during streaming.
         * @param file_path The corresponding file path.
         * @param batch The batch of parsed log entries.
         */
        auto entry_batch_parsed(const QString& file_path, const QVector<LogEntry>& batch) -> void;

        /**
         * @brief Emitted to report progress of the current streaming load.
         * @param file_path The file path being read.
         * @param bytes_read The number of bytes read so far.
         * @param total_bytes The total size of the file in bytes.
         */
        auto progress(const QString& file_path, qint64 bytes_read, qint64 total_bytes) -> void;

        /**
         * @brief Emitted when the current streaming load has finished (successfully or cancelled).
         * @param file_path The file path that finished.
         */
        auto finished(const QString& file_path) -> void;

        /**
         * @brief Emitted when an error occurs during streaming load.
         * @param file_path The file that caused the error.
         * @param message The error message.
         */
        auto error(const QString& file_path, const QString& message) -> void;

        /**
         * @brief Emitted after the streaming thread has fully stopped and internal pointers
         *        are cleared. Safe point to start the next queued file.
         */
        auto streaming_idle() -> void;

    private:
        LogParser m_parser;
        LogStreamWorker* m_worker = nullptr;
        QThread* m_worker_thread = nullptr;
};
