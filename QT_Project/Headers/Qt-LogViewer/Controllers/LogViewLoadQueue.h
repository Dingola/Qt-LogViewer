#pragma once

#include <QList>
#include <QPair>
#include <QString>
#include <QUuid>

/**
 * @file LogViewLoadQueue.h
 * @brief Declares the LogViewLoadQueue which coordinates per-view async log streaming.
 */

class LogLoadingService;

/**
 * @class LogViewLoadQueue
 * @brief Coordinates streaming order across views. Holds a global queue of `(view_id, file_path)`,
 *        tracks the single active stream, and provides cancel/clear helpers.
 *
 * Responsibilities:
 * - Enqueue streaming requests for specific views.
 * - Start the next stream when idle using `LogLoadingService`.
 * - Track the currently active `(view_id, file_path, batch_size)`.
 * - Cancel the active stream by view and clear pending requests per view.
 *
 * Usage:
 * - Call `enqueue(view_id, path)` for each file to stream.
 * - Call `try_start_next(loader, batch_size)` when idle or on `streaming_idle`.
 * - On `finished(file_path)`, call `clear_active_if(file_path)`.
 * - On view removal or cancel, call `cancel_if_active(loader, view_id)` to stop active and purge
 * pending.
 */
class LogViewLoadQueue
{
    public:
        /**
         * @brief Enqueues a file to be streamed for a specific view.
         *
         * Idempotent per `(view_id, file_path)`: if the same pair is already pending or currently
         * active, this method is a no-op to avoid duplicates causing the same file to start again.
         *
         * @param view_id Target view id.
         * @param file_path Absolute file path.
         */
        auto enqueue(const QUuid& view_id, const QString& file_path) -> void;

        /**
         * @brief Attempts to start the next async stream if none is active.
         * @param loader Loader service to start the stream.
         * @param batch_size Entries per emitted batch for the next stream.
         * @return True if a new stream started; false otherwise.
         */
        [[nodiscard]] auto try_start_next(LogLoadingService* loader, qsizetype batch_size) -> bool;

        /**
         * @brief Clears all pending items for the specified view.
         * @param view_id Target view id.
         */
        auto clear_pending_for_view(const QUuid& view_id) -> void;

        /**
         * @brief Cancels the active stream if it belongs to the specified view and clears pendings.
         * @param loader Loader service used to cancel the active stream if needed.
         * @param view_id Target view id.
         */
        auto cancel_if_active(LogLoadingService* loader, const QUuid& view_id) -> void;

        /**
         * @brief Clears the active state if the finished file matches the current active file.
         * @param file_path File path reported as finished.
         */
        auto clear_active_if(const QString& file_path) -> void;

        /**
         * @brief Unconditionally clears the active stream state.
         *
         * Use this when a definitive "idle" signal is received from the loader/service,
         * regardless of file path normalization differences.
         */
        auto clear_active() -> void;

        /**
         * @brief Returns the active view id (empty if none).
         * @return The currently active view id, or a null QUuid if idle.
         */
        [[nodiscard]] auto get_active_view_id() const -> QUuid;

        /**
         * @brief Returns the active file path (empty if none).
         * @return The currently active file path, or empty if idle.
         */
        [[nodiscard]] auto get_active_file_path() const -> QString;

        /**
         * @brief Returns the number of pending items in the queue.
         * @return Count of pending (view_id, file_path) pairs.
         */
        [[nodiscard]] auto get_pending_count() const -> int;

        /**
         * @brief Returns the active batch size (only meaningful while active).
         * @return The batch size used for the active stream, or the last set value.
         */
        [[nodiscard]] auto get_active_batch_size() const -> qsizetype;

    private:
        QList<QPair<QUuid, QString>> m_queue;
        QUuid m_active_view_id;
        QString m_active_file_path;
        qsizetype m_active_batch_size{1000};
};
