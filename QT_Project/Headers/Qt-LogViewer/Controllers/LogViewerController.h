#pragma once

#include <QList>
#include <QObject>
#include <QSet>
#include <QString>
#include <QVector>

#include "Qt-LogViewer/Models/LogFileInfo.h"
#include "Qt-LogViewer/Models/LogFileTreeModel.h"
#include "Qt-LogViewer/Models/LogFilterProxyModel.h"
#include "Qt-LogViewer/Models/LogModel.h"
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
         * @brief Loads log files from the specified file paths.
         * @param file_paths A vector of file paths to load logs from.
         *
         * This method uses the LogLoader service to parse log files and populate the LogModel.
         * It also updates the LogFileTreeModel with information about the loaded log files.
         */
        auto load_logs(const QVector<QString>& file_paths) -> void;

        /**
         * @brief Sets the application name filter.
         * @param app_name The application name to filter by.
         */
        auto set_app_name_filter(const QString& app_name) -> void;

        /**
         * @brief Sets the log level filter.
         * @param levels The set of log levels.
         */
        auto set_level_filter(const QSet<QString>& levels) -> void;

        /**
         * @brief Sets the search filter.
         * @param search_text The search string or regex.
         * @param field The field to search in.
         * @param use_regex Whether to use regex.
         */
        auto set_search_filter(const QString& search_text, const QString& field,
                               bool use_regex) -> void;

        /**
         * @brief Returns the LogModel.
         */
        [[nodiscard]] auto get_log_model() -> LogModel*;

        /**
         * @brief Returns the LogFilterProxyModel.
         */
        [[nodiscard]] auto get_proxy_model() -> LogFilterProxyModel*;

        /**
         * @brief Returns the set of unique application names from the loaded logs.
         * @return A set of application names.
         */
        [[nodiscard]] auto get_app_names() const -> QSet<QString>;

        /**
         * @brief Returns the LogFileTreeModel.
         *
         * This model provides a hierarchical view of log files and their applications.
         */
        [[nodiscard]] auto get_log_file_tree_model() -> LogFileTreeModel*;

    private:
        /**
         * @brief Adds a single log file to the model.
         * @param file The LogFileInfo object to add.
         */
        auto add_log_file(const LogFileInfo& file) -> void;

        /**
         * @brief Adds multiple log files to the model.
         * @param files The list of LogFileInfo objects to add.
         */
        auto add_log_files(const QList<LogFileInfo>& files) -> void;

        /**
         * @brief Checks if a log file with the given file path is already loaded.
         * @param file_path The file path to check.
         * @return True if the file is already loaded, false otherwise.
         */
        auto is_file_loaded(const QString& file_path) const -> bool;

    private:
        LogModel* m_log_model;
        LogFileTreeModel* m_file_tree_model;
        LogFilterProxyModel* m_proxy_model;
        LogLoader m_loader;
        QList<LogFileInfo> m_loaded_log_files;
};
