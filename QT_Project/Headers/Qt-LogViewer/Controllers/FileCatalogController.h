#pragma once

#include <QObject>
#include <QString>
#include <QVector>

#include "Qt-LogViewer/Controllers/LogIngestController.h"
#include "Qt-LogViewer/Models/LogFileInfo.h"
#include "Qt-LogViewer/Models/LogFileTreeModel.h"

/**
 * @file FileCatalogController.h
 * @brief Controller responsible for managing the log file catalog (tree model).
 *
 * Responsibilities:
 * - Own a `LogFileTreeModel` used by the UI.
 * - Add files by probing app name via `LogIngestController::read_first_log_entry` or
 *   falling back to `LogLoader::identify_app`.
 * - Add multiple files by looping over `add_file`.
 * - Remove files from the catalog model.
 */
class FileCatalogController: public QObject
{
        Q_OBJECT

    public:
        /**
         * @brief Construct a new FileCatalogController.
         * @param ingest Non-owning pointer to the ingest controller used to peek first entry.
         * @param parent Optional QObject parent.
         */
        explicit FileCatalogController(LogIngestController* ingest, QObject* parent = nullptr);

        /**
         * @brief Add a single log file into the catalog.
         * @param file_path Absolute path to the log file.
         *
         * Logic:
         * - Try to read the first entry to extract `app_name`.
         * - If unavailable, fall back to `LogLoader::identify_app(file_path)`.
         * - Add resulting `LogFileInfo` to the tree model.
         */
        auto add_file(const QString& file_path) -> void;

        /**
         * @brief Add multiple log files into the catalog.
         * @param file_paths Absolute paths to the log files.
         *
         * Loops over `add_file(file_path)`.
         */
        auto add_files(const QVector<QString>& file_paths) -> void;

        /**
         * @brief Add a single log file into a specific session.
         * @param session_id The session identifier.
         * @param file_path Absolute path to the log file.
         */
        auto add_file_to_session(const QString& session_id, const QString& file_path) -> void;

        /**
         * @brief Add multiple log files into a specific session.
         * @param session_id The session identifier.
         * @param file_paths Absolute paths to the log files.
         */
        auto add_files_to_session(const QString& session_id,
                                  const QVector<QString>& file_paths) -> void;

        /**
         * @brief Remove a single log file from the catalog.
         * @param file_info File info to remove.
         */
        auto remove_file(const LogFileInfo& file_info) -> void;

        /**
         * @brief Get the underlying tree model used by the UI.
         * @return Pointer to `LogFileTreeModel`.
         */
        [[nodiscard]] auto get_model() -> LogFileTreeModel*;

    private:
        LogFileTreeModel* m_model{nullptr};
        LogIngestController* m_ingest{nullptr};  // non-owning
};
