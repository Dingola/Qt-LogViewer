/**
 * @file HistoryWriteService.cpp
 * @brief Implements ordered background history writes and deterministic thread shutdown.
 */

#include "Qt-LogViewer/Services/HistoryWriteService.h"

#include <QMetaObject>
#include <QThread>
#include <utility>

#include "Qt-LogViewer/Services/LogHistoryWriter.h"

/**
 * @brief Creates and starts a background writer for the supplied database.
 * @param database_path SQLite database path shared with the read-side history service.
 * @param parent Optional QObject parent.
 */
HistoryWriteService::HistoryWriteService(QString database_path, QObject* parent)
    : QObject(parent),
      m_thread(new QThread(this)),
      m_writer(new LogHistoryWriter(std::move(database_path)))
{
    m_writer->moveToThread(m_thread);

    connect(m_thread, &QThread::finished, m_writer, &QObject::deleteLater);
    connect(m_writer, &LogHistoryWriter::import_write_finished, this,
            &HistoryWriteService::import_write_finished, Qt::QueuedConnection);

    m_thread->start();
    m_accepting_work = true;
}

/**
 * @brief Waits for queued work and stops the writer thread.
 */
HistoryWriteService::~HistoryWriteService()
{
    shutdown();
}

/**
 * @brief Queues one parsed batch for ordered background storage.
 * @param view_id View that owns the entries.
 * @param file_path Source file identifying the import operation.
 * @param entries Parsed entries to store.
 * @return True when the operation was accepted.
 */
auto HistoryWriteService::store_batch(const QUuid& view_id, const QString& file_path,
                                      const QVector<LogEntry>& entries) -> bool
{
    const bool accepted = is_running();

    if (accepted)
    {
        LogHistoryWriter* writer = m_writer;
        QMetaObject::invokeMethod(
            writer,
            [writer, view_id, file_path, entries]() {
                writer->store_batch(view_id, file_path, entries);
            },
            Qt::QueuedConnection);
    }

    return accepted;
}

/**
 * @brief Queues an import completion marker behind earlier batches.
 * @param view_id View that owns the import.
 * @param file_path Imported source file.
 * @return True when the completion marker was accepted.
 */
auto HistoryWriteService::finish_import(const QUuid& view_id, const QString& file_path) -> bool
{
    const bool accepted = is_running();

    if (accepted)
    {
        LogHistoryWriter* writer = m_writer;
        QMetaObject::invokeMethod(
            writer, [writer, view_id, file_path]() { writer->finish_import(view_id, file_path); },
            Qt::QueuedConnection);
    }

    return accepted;
}

/**
 * @brief Queues cleanup for a discarded view behind earlier writes.
 * @param view_id Discarded view.
 * @return True when the cleanup operation was accepted.
 */
auto HistoryWriteService::discard_view(const QUuid& view_id) -> bool
{
    const bool accepted = is_running();

    if (accepted)
    {
        LogHistoryWriter* writer = m_writer;
        QMetaObject::invokeMethod(
            writer, [writer, view_id]() { writer->discard_view(view_id); }, Qt::QueuedConnection);
    }

    return accepted;
}

/**
 * @brief Queues cleanup for a discarded file behind earlier writes.
 * @param view_id View that owned the file.
 * @param file_path Discarded source file.
 * @return True when the cleanup operation was accepted.
 */
auto HistoryWriteService::discard_file(const QUuid& view_id, const QString& file_path) -> bool
{
    const bool accepted = is_running();

    if (accepted)
    {
        LogHistoryWriter* writer = m_writer;
        QMetaObject::invokeMethod(
            writer, [writer, view_id, file_path]() { writer->discard_file(view_id, file_path); },
            Qt::QueuedConnection);
    }

    return accepted;
}

/**
 * @brief Stops accepting work, waits for queued operations, and ends the worker thread.
 */
auto HistoryWriteService::shutdown() -> void
{
    m_accepting_work = false;

    if (m_thread != nullptr && m_thread->isRunning())
    {
        if (m_writer != nullptr && QThread::currentThread() != m_thread)
        {
            QMetaObject::invokeMethod(m_writer, []() {}, Qt::BlockingQueuedConnection);
        }

        m_thread->quit();
        m_thread->wait();
        m_writer = nullptr;
    }
}

/**
 * @brief Reports whether the service can currently accept operations.
 * @return True while the worker thread is running and shutdown has not begun.
 */
auto HistoryWriteService::is_running() const -> bool
{
    return m_accepting_work && m_thread != nullptr && m_thread->isRunning() && m_writer != nullptr;
}
