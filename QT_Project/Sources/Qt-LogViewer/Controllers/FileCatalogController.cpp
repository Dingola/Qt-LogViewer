/**
 * @file FileCatalogController.cpp
 * @brief Implements FileCatalogController which manages the log file catalog model.
 */

#include "Qt-LogViewer/Controllers/FileCatalogController.h"

#include "Qt-LogViewer/Controllers/LogIngestController.h"
#include "Qt-LogViewer/Models/LogEntry.h"
#include "Qt-LogViewer/Models/LogFileInfo.h"
#include "Qt-LogViewer/Models/LogFileTreeModel.h"
#include "Qt-LogViewer/Services/LogLoader.h"

/**
 * @brief Construct a new FileCatalogController.
 * @param ingest Non-owning pointer to the ingest controller used to peek first entry.
 * @param parent Optional QObject parent.
 */
FileCatalogController::FileCatalogController(LogIngestController* ingest, QObject* parent)
    : QObject(parent), m_model(new LogFileTreeModel(this)), m_ingest(ingest)
{}

/**
 * @brief Add a single log file into the catalog.
 * @param file_path Absolute path to the log file.
 */
auto FileCatalogController::add_file(const QString& file_path) -> void
{
    LogEntry entry;
    if (m_ingest != nullptr)
    {
        entry = m_ingest->read_first_log_entry(file_path);
    }

    LogFileInfo info;
    if (!entry.get_app_name().isEmpty())
    {
        info = entry.get_file_info();
    }
    else
    {
        const QString app_name = LogLoader::identify_app(file_path);
        info = LogFileInfo(file_path, app_name);
    }

    m_model->add_log_file(info);
}

/**
 * @brief Add multiple log files into the catalog.
 * @param file_paths Absolute paths to the log files.
 */
auto FileCatalogController::add_files(const QVector<QString>& file_paths) -> void
{
    for (const auto& file_path: file_paths)
    {
        add_file(file_path);
    }
}

/**
 * @brief Add a single log file into a specific session.
 * @param session_id The session identifier.
 * @param file_path Absolute path to the log file.
 */
auto FileCatalogController::add_file_to_session(const QString& session_id,
                                                const QString& file_path) -> void
{
    LogEntry entry;
    if (m_ingest != nullptr)
    {
        entry = m_ingest->read_first_log_entry(file_path);
    }

    LogFileInfo info;
    if (!entry.get_app_name().isEmpty())
    {
        info = entry.get_file_info();
    }
    else
    {
        const QString app_name = LogLoader::identify_app(file_path);
        info = LogFileInfo(file_path, app_name);
    }

    m_model->add_log_file(session_id, info);
}

/**
 * @brief Add multiple log files into a specific session.
 * @param session_id The session identifier.
 * @param file_paths Absolute paths to the log files.
 */
auto FileCatalogController::add_files_to_session(const QString& session_id,
                                                 const QVector<QString>& file_paths) -> void
{
    for (const auto& file_path: file_paths)
    {
        add_file_to_session(session_id, file_path);
    }
}

/**
 * @brief Remove a single log file from the catalog.
 * @param file_info File info to remove.
 */
auto FileCatalogController::remove_file(const LogFileInfo& file_info) -> void
{
    m_model->remove_log_file(file_info);
}

/**
 * @brief Get the underlying tree model used by the UI.
 * @return Pointer to `LogFileTreeModel`.
 */
auto FileCatalogController::get_model() -> LogFileTreeModel*
{
    LogFileTreeModel* model = m_model;
    return model;
}
