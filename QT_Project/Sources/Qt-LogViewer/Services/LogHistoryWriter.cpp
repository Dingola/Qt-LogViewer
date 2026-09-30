/**
 * @file LogHistoryWriter.cpp
 * @brief Implements the background writer for asynchronously imported log history.
 */

#include "Qt-LogViewer/Services/LogHistoryWriter.h"

#include <QFileInfo>
#include <utility>

#include "Qt-LogViewer/Services/LogHistoryService.h"

namespace
{
constexpr auto k_history_write_error = "Could not store parsed entries in the log history.";
}

/**
 * @brief Constructs a writer for the supplied SQLite database.
 * @param database_path Path shared with the read-side history service.
 * @param parent Optional QObject parent.
 */
LogHistoryWriter::LogHistoryWriter(QString database_path, QObject* parent)
    : QObject(parent), m_database_path(std::move(database_path))
{}

/**
 * @brief Destroys the writer and its thread-owned history connection.
 */
LogHistoryWriter::~LogHistoryWriter() = default;

/**
 * @brief Stores one parsed batch for an asynchronous import.
 * @param view_id View that owns the imported entries.
 * @param file_path Source file used to identify one import operation.
 * @param entries Parsed entries to store.
 */
auto LogHistoryWriter::store_batch(const QUuid& view_id, const QString& file_path,
                                   const QVector<LogEntry>& entries) -> void
{
    const QString absolute_file_path = QFileInfo(file_path).absoluteFilePath();
    const bool already_failed = m_failed_imports.value(view_id).contains(absolute_file_path);
    const bool can_store = !view_id.isNull() && !absolute_file_path.isEmpty() &&
                           !entries.isEmpty() && !already_failed && ensure_history_service();

    if (can_store && !m_history_service->add_entries(view_id, entries))
    {
        m_failed_imports[view_id].insert(absolute_file_path);
        m_history_service->remove_file_entries(view_id, absolute_file_path);
    }
    else if (!already_failed && !entries.isEmpty() && !can_store)
    {
        m_failed_imports[view_id].insert(absolute_file_path);
    }
}

/**
 * @brief Completes an import after all earlier queued batches were processed.
 * @param operation_id Unique identifier of the completed import attempt.
 * @param view_id View that owns the import.
 * @param file_path Imported source file.
 */
auto LogHistoryWriter::finish_import(const QUuid& operation_id, const QUuid& view_id,
                                     const QString& file_path) -> void
{
    const QString absolute_file_path = QFileInfo(file_path).absoluteFilePath();
    const bool service_available = ensure_history_service();
    const bool write_failed =
        !service_available || m_failed_imports.value(view_id).contains(absolute_file_path);
    const QString error_message =
        write_failed ? QString::fromLatin1(k_history_write_error) : QString();

    clear_failed_import(view_id, absolute_file_path);
    emit import_write_finished(operation_id, view_id, absolute_file_path, !write_failed,
                               error_message);
}

/**
 * @brief Removes entries that may have been queued before a view was discarded.
 * @param view_id Discarded view.
 */
auto LogHistoryWriter::discard_view(const QUuid& view_id) -> void
{
    if (ensure_history_service())
    {
        m_history_service->remove_view_entries(view_id);
    }

    m_failed_imports.remove(view_id);
}

/**
 * @brief Removes entries that may have been queued before a file was discarded.
 * @param view_id View that owned the file.
 * @param file_path Discarded source file.
 */
auto LogHistoryWriter::discard_file(const QUuid& view_id, const QString& file_path) -> void
{
    const QString absolute_file_path = QFileInfo(file_path).absoluteFilePath();

    if (ensure_history_service())
    {
        m_history_service->remove_file_entries(view_id, absolute_file_path);
    }

    clear_failed_import(view_id, absolute_file_path);
}

/**
 * @brief Creates the SQLite service in the writer thread when needed.
 * @return True when the writer connection is available.
 */
auto LogHistoryWriter::ensure_history_service() -> bool
{
    if (m_history_service == nullptr)
    {
        m_history_service = new LogHistoryService(m_database_path, this);
    }

    return m_history_service->is_available();
}

/**
 * @brief Clears the failed state for one import operation.
 * @param view_id View that owns the import.
 * @param file_path Imported source file.
 */
auto LogHistoryWriter::clear_failed_import(const QUuid& view_id, const QString& file_path) -> void
{
    m_failed_imports[view_id].remove(file_path);

    if (m_failed_imports.value(view_id).isEmpty())
    {
        m_failed_imports.remove(view_id);
    }
}
