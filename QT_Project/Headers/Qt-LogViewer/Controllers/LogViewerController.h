#pragma once

#include <QList>
#include <QMap>
#include <QObject>
#include <QSet>
#include <QString>
#include <QUuid>
#include <QVector>

#include "Qt-LogViewer/Models/LogFileInfo.h"
#include "Qt-LogViewer/Models/LogFileTreeModel.h"
#include "Qt-LogViewer/Models/LogModel.h"
#include "Qt-LogViewer/Models/LogSortFilterProxyModel.h"
#include "Qt-LogViewer/Models/PagingProxyModel.h"
#include "Qt-LogViewer/Services/LogLoader.h"

/**
 * @file LogViewerController.h
 * @brief Controller/service class for managing log loading and filtering.
 */
class LogViewerController: public QObject
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs a LogViewerController.
         * @param log_format The log format string for parsing.
         * @param parent The parent QObject.
         */
        explicit LogViewerController(const QString& log_format, QObject* parent = nullptr);

        /**
         * @brief Destroys the LogViewerController.
         */
        ~LogViewerController() override = default;

        /**
         * @brief Sets the current view to the given QUuid if it exists.
         * @param view_id The QUuid of the view to set as current.
         * @return True if the view was set successfully, false if the view_id does not exist.
         */
        auto set_current_view(const QUuid& view_id) -> bool;

        /**
         * @brief Returns the QUuid of the current view.
         * @return The QUuid of the current view.
         */
        [[nodiscard]] auto get_current_view() const -> QUuid;

        /**
         * @brief Removes a view and all associated models and proxies.
         * @param view_id The QUuid of the view to remove.
         * @return True if the view was removed, false if not found.
         */
        auto remove_view(const QUuid& view_id) -> bool;

        /**
         * @brief Adds a single log file to the LogFileTreeModel.
         * @param file_path The path to the log file.
         */
        auto add_log_file_to_tree(const QString& file_path) -> void;

        /**
         * @brief Adds multiple log files to the LogFileTreeModel.
         * @param file_paths The vector of log file paths.
         */
        auto add_log_files_to_tree(const QVector<QString>& file_paths) -> void;

        /**
         * @brief Loads a single log file and creates a new view (model/proxy) for it.
         * @param log_file The LogFileInfo to load and display.
         * @return QUuid of the created view, or an empty QUuid if the file is already loaded.
         */
        auto load_log_file(const QString& file_path) -> QUuid;

        /**
         * @brief Loads log files from the specified file paths and creates a new view (model/proxy)
         * for them.
         * @param file_paths A vector of file paths to load logs from.
         * @return QUuid of the created view, or an empty QUuid if no files were loaded.
         */
        auto load_log_files(const QVector<QString>& file_paths) -> QUuid;

        /**
         * @brief Starts streaming load of a single log file and creates a new view (model/proxy).
         * @param file_path The path to the log file to stream.
         * @param batch_size Number of entries per batch appended to the model.
         * @return QUuid of the created view.
         */
        auto load_log_file_async(const QString& file_path, qsizetype batch_size = 1000) -> QUuid;

        /**
         * @brief Starts streaming load of multiple log files into a single new view (model/proxy).
         *        Files are streamed sequentially in the background.
         * @param file_paths Paths to stream.
         * @param batch_size Number of entries per batch appended to the model.
         * @return QUuid of the created view.
         */
        auto load_log_files_async(const QVector<QString>& file_paths,
                                  qsizetype batch_size = 1000) -> QUuid;

        /**
         * @brief Cancels any ongoing streaming for the specified view and clears its pending queue.
         * @param view_id The view to cancel streaming for.
         */
        auto cancel_loading(const QUuid& view_id) -> void;

        /**
         * @brief Sets the application name filter for the current view.
         * @param app_name The application name to filter by.
         */
        auto set_app_name_filter(const QString& app_name) -> void;

        /**
         * @brief Sets the application name filter for the specified view.
         * @param view_id The QUuid of the view.
         * @param app_name The application name to filter by.
         */
        auto set_app_name_filter(const QUuid& view_id, const QString& app_name) -> void;

        /**
         * @brief Sets the log level filter for the current view.
         * @param levels The set of log levels.
         */
        auto set_log_level_filters(const QSet<QString>& levels) -> void;

        /**
         * @brief Sets the log level filter for the specified view.
         * @param view_id The QUuid of the view.
         * @param levels The set of log levels.
         */
        auto set_log_level_filters(const QUuid& view_id, const QSet<QString>& levels) -> void;

        /**
         * @brief Sets the search filter for the current view.
         * @param search_text The search string or regex.
         * @param field The field to search in.
         * @param use_regex Whether to use regex.
         */
        auto set_search_filter(const QString& search_text, const QString& field,
                               bool use_regex) -> void;

        /**
         * @brief Sets the search filter for the specified view.
         * @param view_id The QUuid of the view.
         * @param search_text The search string or regex.
         * @param field The field to search in.
         * @param use_regex Whether to use regex.
         */
        auto set_search_filter(const QUuid& view_id, const QString& search_text,
                               const QString& field, bool use_regex) -> void;

        /**
         * @brief Returns the LogModel for the current view.
         * @return Pointer to the LogModel.
         */
        [[nodiscard]] auto get_log_model() -> LogModel*;

        /**
         * @brief Returns the LogModel for the specified view.
         * @param view_id The QUuid of the view.
         * @return Pointer to the LogModel, or nullptr if not found.
         */
        [[nodiscard]] auto get_log_model(const QUuid& view_id) -> LogModel*;

        /**
         * @brief Returns the LogSortFilterProxyModel for the current view.
         * @return Pointer to the LogSortFilterProxyModel.
         */
        [[nodiscard]] auto get_sort_filter_proxy() -> LogSortFilterProxyModel*;

        /**
         * @brief Returns the LogSortFilterProxyModel for the specified view.
         * @param view_id The QUuid of the view.
         * @return Pointer to the LogSortFilterProxyModel, or nullptr if not found.
         */
        [[nodiscard]] auto get_sort_filter_proxy(const QUuid& view_id) -> LogSortFilterProxyModel*;

        /**
         * @brief Returns the PagingProxyModel for the current view.
         * @return Pointer to the PagingProxyModel.
         */
        [[nodiscard]] auto get_paging_proxy() -> PagingProxyModel*;

        /**
         * @brief Returns the PagingProxyModel for the specified view.
         * @param view_id The QUuid of the view.
         * @return Pointer to the PagingProxyModel, or nullptr if not found.
         */
        [[nodiscard]] auto get_paging_proxy(const QUuid& view_id) -> PagingProxyModel*;

        /**
         * @brief Returns the set of unique application names from the loaded logs in the current
         * view.
         * @return A set of application names.
         */
        [[nodiscard]] auto get_app_names() const -> QSet<QString>;

        /**
         * @brief Returns the set of unique application names from the loaded logs in the specified
         * view.
         * @param view_id The QUuid of the view.
         * @return A set of application names.
         */
        [[nodiscard]] auto get_app_names(const QUuid& view_id) const -> QSet<QString>;

        /**
         * @brief Returns the current application name filter.
         * @return The application name filter string.
         */
        [[nodiscard]] auto get_app_name_filter() const -> QString;

        /**
         * @brief Returns the application name filter for the specified view.
         * @param view_id The QUuid of the view.
         * @return The application name filter string.
         */
        [[nodiscard]] auto get_app_name_filter(const QUuid& view_id) const -> QString;

        /**
         * @brief Returns the available log levels for the specified view.
         *        Currently returns the same static set for all views.
         * @param view_id The QUuid of the view.
         * @return QVector of log level names (e.g., "Trace", "Debug", ...).
         */
        [[nodiscard]] auto get_available_log_levels(const QUuid& view_id) const -> QVector<QString>;

        /**
         * @brief Returns the current set of log levels being filtered.
         * @return The set of log levels.
         */
        [[nodiscard]] auto get_log_level_filters() const -> QSet<QString>;

        /**
         * @brief Returns a map of log level names to their counts in the specified view.
         * @param view_id The QUuid of the view.
         * @return QMap of log level names to counts.
         */
        [[nodiscard]] auto get_log_level_counts(const QUuid& view_id) const -> QMap<QString, int>;

        /**
         * @brief Returns a map of log level names to their counts in the current view.
         * @return QMap of log level names to counts.
         */
        [[nodiscard]] auto get_log_level_counts() const -> QMap<QString, int>;

        /**
         * @brief Returns the set of log levels being filtered for the specified view.
         * @param view_id The QUuid of the view.
         * @return The set of log levels.
         */
        [[nodiscard]] auto get_log_level_filters(const QUuid& view_id) const -> QSet<QString>;

        /**
         * @brief Returns the current search text.
         * @return The search text string.
         */
        [[nodiscard]] auto get_search_text() const -> QString;

        /**
         * @brief Returns the search text for the specified view.
         * @param view_id The QUuid of the view.
         * @return The search text string.
         */
        [[nodiscard]] auto get_search_text(const QUuid& view_id) const -> QString;

        /**
         * @brief Returns the current search field.
         * @return The search field string.
         */
        [[nodiscard]] auto get_search_field() const -> QString;

        /**
         * @brief Returns the search field for the specified view.
         * @param view_id The QUuid of the view.
         * @return The search field string.
         */
        [[nodiscard]] auto get_search_field(const QUuid& view_id) const -> QString;

        /**
         * @brief Returns whether the search text is treated as a regex.
         * @return True if using regex, false if plain text.
         */
        [[nodiscard]] auto is_search_regex() const -> bool;

        /**
         * @brief Returns whether the search text is treated as a regex for the specified view.
         * @param view_id The QUuid of the view.
         * @return True if using regex, false if plain text.
         */
        [[nodiscard]] auto is_search_regex(const QUuid& view_id) const -> bool;

        /**
         * @brief Returns the LogFileTreeModel.
         *
         * This model provides a hierarchical view of log files and their applications.
         * @return Pointer to the LogFileTreeModel.
         */
        [[nodiscard]] auto get_log_file_tree_model() -> LogFileTreeModel*;

        /**
         * @brief Returns all log entries in the current view.
         * @return QVector<LogEntry> containing all entries.
         */
        [[nodiscard]] auto get_log_entries() const -> QVector<LogEntry>;

        /**
         * @brief Returns all log entries in the specified view.
         * @param view_id The QUuid of the view.
         * @return QVector<LogEntry> containing all entries.
         */
        [[nodiscard]] auto get_log_entries(const QUuid& view_id) const -> QVector<LogEntry>;

        /**
         * @brief Returns all log entries for a given file in the current view.
         * @param file_info The LogFileInfo for the file.
         * @return QVector<LogEntry> containing all entries for the file.
         */
        [[nodiscard]] auto get_entries_for_file(const LogFileInfo& file_info) -> QVector<LogEntry>;

        /**
         * @brief Returns all log entries for a given file in the specified view.
         * @param view_id The QUuid of the view.
         * @param file_info The LogFileInfo for the file.
         * @return QVector<LogEntry> containing all entries for the file.
         */
        [[nodiscard]] auto get_entries_for_file(const QUuid& view_id,
                                                const LogFileInfo& file_info) -> QVector<LogEntry>;

        /**
         * @brief Checks if a log file with the given file path is already loaded.
         * @param file_path The file path to check.
         * @return True if the file is already loaded, false otherwise.
         */
        [[nodiscard]] auto is_file_loaded(const QString& file_path) const -> bool;

        /**
         * @brief Checks if a log file with the given file path is loaded in the specified view.
         * @param view_id The QUuid of the view.
         * @param file_path The file path to check.
         * @return True if the file is loaded in the view, false otherwise.
         */
        [[nodiscard]] auto is_file_loaded(const QUuid& view_id,
                                          const QString& file_path) const -> bool;

    signals:
        /**
         * @brief Signal emitted when the current view ID changes.
         */
        auto current_view_id_changed(const QUuid& view_id) -> void;

        /**
         * @brief Signal emitted when a view is removed (e.g., after last log file is deleted).
         * @param view_id The QUuid of the removed view.
         */
        auto view_removed(const QUuid& view_id) -> void;

        /**
         * @brief Streaming progress signal mapped to view IDs.
         * @param view_id The view receiving streamed data.
         * @param bytes_read Bytes read so far.
         * @param total_bytes Total file size.
         */
        auto loading_progress(const QUuid& view_id, qint64 bytes_read, qint64 total_bytes) -> void;

        /**
         * @brief Emitted when a file finishes streaming for a view.
         * @param view_id View that received the data.
         * @param file_path File that finished.
         */
        auto loading_finished(const QUuid& view_id, const QString& file_path) -> void;

        /**
         * @brief Emitted when an error occurs during streaming for a view.
         * @param view_id View that attempted to load the file.
         * @param file_path File that errored.
         * @param message Error message.
         */
        auto loading_error(const QUuid& view_id, const QString& file_path,
                           const QString& message) -> void;

    public slots:
        /**
         * @brief Removes a single log file from all views and from the LogFileTreeModel.
         *        If a view becomes empty, it is deleted and view_removed() is emitted.
         * @param file The LogFileInfo object to remove.
         */
        auto remove_log_file(const LogFileInfo& file) -> void;

    private:
        /**
         * @brief Enqueues an asynchronous load request for a log file.
         * @param view_id The QUuid of the view to load into.
         * @param file_path The path to the log file.
         */
        auto enqueue_async(const QUuid& view_id, const QString& file_path) -> void;

        /**
         * @brief Attempts to start the next asynchronous load if none is active.
         * @param batch_size Number of entries per batch.
         */
        auto try_start_next_async(qsizetype batch_size) -> void;

        /**
         * @brief Callback when an asynchronous load batch is received.
         * @param view_id The QUuid of the view receiving data.
         * @param file_path The path to the log file.
         * @param entries The batch of log entries.
         */
        auto clear_pending_for_view(const QUuid& view_id) -> void;

        /**
         * @brief Ensures that models and proxies exist for the specified view ID.
         * @param view_id The QUuid of the view.
         */
        auto ensure_view_models(const QUuid& view_id) -> void;

    private:
        LogFileTreeModel* m_file_tree_model;
        LogLoader m_loader;
        QMap<QUuid, QList<LogFileInfo>> m_loaded_log_files;
        QMap<QUuid, LogModel*> m_view_models;
        QMap<QUuid, LogSortFilterProxyModel*> m_sort_filter_models;
        QMap<QUuid, PagingProxyModel*> m_paging_models;
        QUuid m_current_view_id;

        // Async orchestration (single active stream globally, queued requests)
        QList<QPair<QUuid, QString>> m_async_queue;
        QUuid m_active_view_id;
        QString m_active_file_path;
        qsizetype m_active_batch_size;
};
