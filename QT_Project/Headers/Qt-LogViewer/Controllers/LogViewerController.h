#pragma once

#include <QObject>
#include <QSet>
#include <QString>
#include <QVector>

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
         * @brief Loads log files and updates the model.
         * @param file_paths The list of log file paths.
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
        auto set_search_filter(const QString& search_text, const QString& field, bool use_regex) -> void;

        /**
         * @brief Returns the LogModel.
         */
        [[nodiscard]] auto get_log_model() -> LogModel*;

        /**
         * @brief Returns the LogFilterProxyModel.
         */
        [[nodiscard]] auto get_proxy_model() -> LogFilterProxyModel*;

    private:
        LogModel m_log_model;
        LogFilterProxyModel m_proxy_model;
        LogLoader m_loader;
};
