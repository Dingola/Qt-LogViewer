#pragma once

#include <QByteArray>
#include <QFileSystemWatcher>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QUuid>
#include <QVector>

#include "Qt-LogViewer/Models/LogEntry.h"
#include "Qt-LogViewer/Services/LogParser.h"

/**
 * @file LogTailerService.h
 * @brief Declares incremental live-log tailing based on QFileSystemWatcher.
 */

/**
 * @class LogTailerService
 * @brief Watches loaded files and emits newly appended, complete log records.
 *
 * Each registration retains a byte offset and a partial-line buffer. File and parent-directory
 * changes are debounced before reading. A stable prefix fingerprint detects replacement or
 * rotation even when the replacement file is not smaller than the previous file.
 *
 * The initial offset is the current end of the file. Consequently, enabling live tail does not
 * duplicate entries that were already loaded by the normal loading pipeline.
 */
class LogTailerService final: public QObject
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs the live-tail service.
         * @param log_format Log format used to parse appended lines.
         * @param parent Optional QObject parent.
         */
        explicit LogTailerService(const QString& log_format, QObject* parent = nullptr);

        /**
         * @brief Starts tailing a file for one view from the current file end.
         * @param view_id Target view identifier.
         * @param file_path Absolute or relative log-file path.
         */
        auto start_tailing(const QUuid& view_id, const QString& file_path) -> void;

        /**
         * @brief Stops tailing one file for one view.
         * @param view_id Target view identifier.
         * @param file_path Absolute or relative log-file path.
         */
        auto stop_tailing(const QUuid& view_id, const QString& file_path) -> void;

        /**
         * @brief Stops every tail registration belonging to a view.
         * @param view_id Target view identifier.
         */
        auto stop_tailing_view(const QUuid& view_id) -> void;

        /**
         * @brief Stops every active tail registration.
         *
         * This method is intended for controlled application shutdown.
         */
        auto stop_all_tailing() -> void;

        /**
         * @brief Sets the single-shot debounce interval used after watcher notifications.
         * @param debounce_interval_ms Debounce interval in milliseconds; negative values become
         * zero.
         */
        auto set_debounce_interval_ms(int debounce_interval_ms) -> void;

    signals:
        /**
         * @brief Emits parsed records appended to a watched file.
         * @param view_id Receiving view identifier.
         * @param file_path Absolute source file path.
         * @param entries Parsed complete records.
         */
        void entries_available(const QUuid& view_id, const QString& file_path,
                               const QVector<LogEntry>& entries);

    private slots:
        /**
         * @brief Schedules processing after a watched file changes.
         * @param path Changed file path.
         */
        auto handle_file_changed(const QString& path) -> void;

        /**
         * @brief Schedules processing after a watched parent directory changes.
         * @param path Changed directory path.
         */
        auto handle_directory_changed(const QString& path) -> void;

        /**
         * @brief Processes all registrations after the debounce interval expires.
         */
        auto process_pending_changes() -> void;

    private:
        /**
         * @brief Stores state for one view/file tail registration.
         */
        struct TailRegistration {
                QUuid view_id;
                QString file_path;
                qint64 offset{0};
                QByteArray incomplete_line;
                QByteArray initial_prefix_fingerprint;
        };

        /**
         * @brief Ensures the file and its parent directory are watched where possible.
         * @param file_path Absolute file path.
         */
        auto ensure_watches(const QString& file_path) -> void;

        /**
         * @brief Reads, parses, and emits newly complete records for one registration.
         * @param registration Registration state to update.
         */
        auto process_registration(TailRegistration& registration) -> void;

        /**
         * @brief Reads a stable prefix used to distinguish replacement from append.
         * @param file_path Absolute file path.
         * @return The first up to 4096 bytes, or an empty array if the file cannot be read.
         */
        [[nodiscard]] static auto get_prefix_fingerprint(const QString& file_path) -> QByteArray;

        /**
         * @brief Determines whether a registration already exists.
         * @param view_id Target view identifier.
         * @param file_path Absolute file path.
         * @return True if the file is already tailed for the view.
         */
        [[nodiscard]] auto has_registration(const QUuid& view_id,
                                            const QString& file_path) const -> bool;

    private:
        QFileSystemWatcher m_watcher;
        QTimer m_debounce_timer;
        LogParser m_parser;
        QVector<TailRegistration> m_registrations;
};
