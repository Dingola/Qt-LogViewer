/**
 * @file HistoryWriteService.cpp
 * @brief Implements coalesced, cancellation-aware background history writes.
 */

#include "Qt-LogViewer/Services/HistoryWriteService.h"

#include <QFileInfo>
#include <QList>
#include <QMetaObject>
#include <QMutexLocker>
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
    connect(
        m_writer, &LogHistoryWriter::import_write_finished, this,
        [this](const QUuid& operation_id, const QUuid& view_id, const QString& file_path,
               bool succeeded, const QString& error_message) {
            const bool operation_is_active = m_operations.remove(operation_id);
            if (operation_is_active)
            {
                emit import_write_finished(operation_id, view_id, file_path, succeeded,
                                           error_message);
            }
        },
        Qt::QueuedConnection);

    m_thread->start();
    m_accepting_work = true;
}

/** @brief Waits for queued work and stops the writer thread. */
HistoryWriteService::~HistoryWriteService()
{
    shutdown();
}

/**
 * @brief Registers one uniquely identified asynchronous import.
 * @param operation_id Unique identifier of this import attempt.
 * @param view_id View receiving the imported entries.
 * @param file_path Source file belonging to the import attempt.
 * @param cache_generation Optional building cache generation populated by the writer.
 * @return True when the operation was registered.
 */
auto HistoryWriteService::begin_import(const QUuid& operation_id, const QUuid& view_id,
                                       const QString& file_path,
                                       std::optional<LogCacheGeneration> cache_generation) -> bool
{
    const QString absolute_file_path = QFileInfo(file_path).absoluteFilePath();
    const bool registered = is_running() && !operation_id.isNull() && !view_id.isNull() &&
                            !absolute_file_path.isEmpty() && !m_operations.contains(operation_id);

    if (registered)
    {
        auto operation = std::make_shared<ImportOperation>();
        operation->view_id = view_id;
        operation->file_path = absolute_file_path;
        operation->cache_generation = std::move(cache_generation);
        m_operations.insert(operation_id, operation);

        LogHistoryWriter* writer = m_writer;
        const std::optional<LogCacheGeneration> writer_generation = operation->cache_generation;
        QMetaObject::invokeMethod(
            writer,
            [writer, operation_id, writer_generation]() {
                writer->begin_import(operation_id, writer_generation);
            },
            Qt::QueuedConnection);
    }

    return registered;
}

/**
 * @brief Adds one parsed batch to the operation's coalesced writer buffer.
 * @param operation_id Unique identifier of the import attempt.
 * @param view_id View that owns the entries.
 * @param file_path Source file identifying the import operation.
 * @param entries Parsed entries to store.
 * @return True when the operation accepted the batch.
 */
auto HistoryWriteService::store_batch(const QUuid& operation_id, const QUuid& view_id,
                                      const QString& file_path,
                                      const QVector<LogEntry>& entries) -> bool
{
    const auto iterator = m_operations.constFind(operation_id);
    const std::shared_ptr<ImportOperation> operation =
        iterator != m_operations.cend() ? iterator.value() : nullptr;
    bool accepted = is_running() && operation != nullptr && operation->view_id == view_id &&
                    operation->file_path == QFileInfo(file_path).absoluteFilePath() &&
                    !entries.isEmpty();
    bool schedule_drain = false;

    if (accepted)
    {
        QMutexLocker locker(&operation->mutex);
        accepted =
            !operation->cancelled.load(std::memory_order_acquire) && !operation->finish_requested;
        if (accepted)
        {
            operation->pending_entries.append(entries);
            ++operation->pending_batch_count;
            if (!operation->drain_scheduled)
            {
                operation->drain_scheduled = true;
                schedule_drain = true;
            }
        }
    }

    if (schedule_drain)
    {
        queue_drain(operation_id, operation);
    }

    return accepted;
}

/**
 * @brief Requests completion after every buffered batch has been stored.
 * @param operation_id Unique identifier of the import attempt.
 * @param view_id View that owns the import.
 * @param file_path Imported source file.
 * @return True when the completion request was accepted.
 */
auto HistoryWriteService::finish_import(const QUuid& operation_id, const QUuid& view_id,
                                        const QString& file_path) -> bool
{
    const auto iterator = m_operations.constFind(operation_id);
    const std::shared_ptr<ImportOperation> operation =
        iterator != m_operations.cend() ? iterator.value() : nullptr;
    bool accepted = is_running() && operation != nullptr && operation->view_id == view_id &&
                    operation->file_path == QFileInfo(file_path).absoluteFilePath();
    bool schedule_drain = false;

    if (accepted)
    {
        QMutexLocker locker(&operation->mutex);
        accepted =
            !operation->cancelled.load(std::memory_order_acquire) && !operation->finish_requested;
        if (accepted)
        {
            operation->finish_requested = true;
            if (!operation->drain_scheduled)
            {
                operation->drain_scheduled = true;
                schedule_drain = true;
            }
        }
    }

    if (schedule_drain)
    {
        queue_drain(operation_id, operation);
    }

    return accepted;
}

/**
 * @brief Immediately marks every import belonging to a view as cancelled.
 * @param view_id View whose buffered and queued writer work must be skipped.
 */
auto HistoryWriteService::cancel_view(const QUuid& view_id) -> void
{
    QList<QUuid> cancelled_operation_ids;

    for (auto iterator = m_operations.cbegin(); iterator != m_operations.cend(); ++iterator)
    {
        const std::shared_ptr<ImportOperation>& operation = iterator.value();
        if (operation->view_id == view_id)
        {
            operation->cancelled.store(true, std::memory_order_release);
            QMutexLocker locker(&operation->mutex);
            operation->pending_entries.clear();
            operation->pending_batch_count = 0;
            cancelled_operation_ids.append(iterator.key());
        }
    }

    for (const QUuid& operation_id: cancelled_operation_ids)
    {
        LogHistoryWriter* writer = m_writer;
        if (writer != nullptr)
        {
            QMetaObject::invokeMethod(
                writer, [writer, operation_id]() { writer->cancel_import(operation_id); },
                Qt::QueuedConnection);
        }
        m_operations.remove(operation_id);
    }
}

/**
 * @brief Queues cleanup for a discarded view behind an in-progress writer call.
 * @param view_id Discarded view.
 * @return True when the cleanup operation was accepted.
 */
auto HistoryWriteService::discard_view(const QUuid& view_id) -> bool
{
    cancel_view(view_id);
    const bool accepted = is_running() && !view_id.isNull();

    if (accepted)
    {
        LogHistoryWriter* writer = m_writer;
        QMetaObject::invokeMethod(
            writer, [writer, view_id]() { writer->discard_view(view_id); }, Qt::QueuedConnection);
    }

    return accepted;
}

/**
 * @brief Queues cleanup for a discarded file behind an in-progress writer call.
 * @param view_id View that owned the file.
 * @param file_path Discarded source file.
 * @return True when the cleanup operation was accepted.
 */
auto HistoryWriteService::discard_file(const QUuid& view_id, const QString& file_path) -> bool
{
    const QString absolute_file_path = QFileInfo(file_path).absoluteFilePath();
    QList<QUuid> cancelled_operation_ids;

    for (auto iterator = m_operations.cbegin(); iterator != m_operations.cend(); ++iterator)
    {
        const std::shared_ptr<ImportOperation>& operation = iterator.value();
        if (operation->view_id == view_id && operation->file_path == absolute_file_path)
        {
            operation->cancelled.store(true, std::memory_order_release);
            QMutexLocker locker(&operation->mutex);
            operation->pending_entries.clear();
            operation->pending_batch_count = 0;
            cancelled_operation_ids.append(iterator.key());
        }
    }

    for (const QUuid& operation_id: cancelled_operation_ids)
    {
        LogHistoryWriter* writer = m_writer;
        if (writer != nullptr)
        {
            QMetaObject::invokeMethod(
                writer, [writer, operation_id]() { writer->cancel_import(operation_id); },
                Qt::QueuedConnection);
        }
        m_operations.remove(operation_id);
    }

    const bool accepted = is_running() && !view_id.isNull() && !absolute_file_path.isEmpty();

    if (accepted)
    {
        LogHistoryWriter* writer = m_writer;
        QMetaObject::invokeMethod(
            writer,
            [writer, view_id, absolute_file_path]() {
                writer->discard_file(view_id, absolute_file_path);
            },
            Qt::QueuedConnection);
    }

    return accepted;
}

/** @brief Stops accepting work, waits for queued operations, and ends the worker thread. */
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

    m_operations.clear();
}

/**
 * @brief Reports whether the service can currently accept operations.
 * @return True while the worker thread is running and shutdown has not begun.
 */
auto HistoryWriteService::is_running() const -> bool
{
    return m_accepting_work && m_thread != nullptr && m_thread->isRunning() && m_writer != nullptr;
}

/**
 * @brief Queues the sole drain task for one import operation.
 * @param operation_id Unique identifier forwarded with writer completion.
 * @param operation Shared operation state retained by the queued task.
 */
auto HistoryWriteService::queue_drain(const QUuid& operation_id,
                                      const std::shared_ptr<ImportOperation>& operation) -> void
{
    LogHistoryWriter* writer = m_writer;
    HistoryWriteService* service = this;
    QMetaObject::invokeMethod(
        writer,
        [writer, service, operation_id, operation]() {
            bool draining = true;
            while (draining)
            {
                QVector<LogEntry> entries;
                qsizetype batch_count = 0;
                bool finish_import = false;

                {
                    QMutexLocker locker(&operation->mutex);
                    if (operation->cancelled.load(std::memory_order_acquire))
                    {
                        operation->pending_entries.clear();
                        operation->pending_batch_count = 0;
                        operation->drain_scheduled = false;
                        draining = false;
                    }
                    else if (!operation->pending_entries.isEmpty())
                    {
                        entries.swap(operation->pending_entries);
                        batch_count = std::exchange(operation->pending_batch_count, 0);
                    }
                    else
                    {
                        finish_import = operation->finish_requested;
                        operation->drain_scheduled = false;
                        draining = false;
                    }
                }

                if (!entries.isEmpty() && !operation->cancelled.load(std::memory_order_acquire))
                {
                    writer->store_batch(operation_id, operation->view_id, operation->file_path,
                                        entries);
                    QMetaObject::invokeMethod(
                        service,
                        [service, operation_id, batch_count]() {
                            emit service->batches_processed(operation_id, batch_count);
                        },
                        Qt::QueuedConnection);
                }

                if (finish_import && !operation->cancelled.load(std::memory_order_acquire))
                {
                    writer->finish_import(operation_id, operation->view_id, operation->file_path);
                }
            }
        },
        Qt::QueuedConnection);
}
