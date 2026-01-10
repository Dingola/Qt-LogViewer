#pragma once

#include <QList>
#include <QMap>
#include <QObject>
#include <QString>
#include <QUuid>
#include <QVector>

class FilterCoordinator;

#include "Qt-LogViewer/Controllers/LogViewContext.h"
#include "Qt-LogViewer/Models/LogEntry.h"
#include "Qt-LogViewer/Models/LogFileInfo.h"
#include "Qt-LogViewer/Models/SessionTypes.h"

/**
 * @file ViewRegistry.h
 * @brief Registry that owns and tracks `LogViewContext` instances and the current view id.
 *
 * Responsibilities:
 * - Create / remove per-view contexts.
 * - Provide lookup helpers for contexts, entries and loaded file paths.
 * - Mutate per-view loaded-files and remove per-view entries by file path.
 * - Emit lifecycle signals consumed by the facade (LogViewerController).
 *
 * Serialization helpers:
 * - `export_view_state()` captures a view's loaded files, filters, paging and sorting into
 *   `SessionViewState` with a tab-title suggestion. It provides round-trip safety with
 *   `import_view_state()` when applied onto a compatible environment.
 */
class ViewRegistry: public QObject
{
        Q_OBJECT

    public:
        /**
         * @brief Construct a new ViewRegistry.
         * @param parent Optional QObject parent for Qt ownership.
         */
        explicit ViewRegistry(QObject* parent = nullptr);

        /**
         * @brief Destroy the ViewRegistry.
         *
         * Clears internal maps. Owned `LogViewContext` objects are parented to this object
         * and will be deleted by Qt object hierarchy cleanup.
         */
        ~ViewRegistry() override;

        /**
         * @brief Create a new view context with a generated id and return it.
         * @return QUuid Created view id.
         */
        [[nodiscard]] auto create_view() -> QUuid;

        /**
         * @brief Create a new view context under the given fixed id.
         * @param view_id The explicit id to use.
         * @return True if created; false if it already existed.
         */
        auto create_view(const QUuid& view_id) -> bool;

        /**
         * @brief Ensures a view exists for the given id; creates it if missing.
         * @param view_id The id to ensure.
         */
        auto ensure_view(const QUuid& view_id) -> void;

        /**
         * @brief Remove a view context and free resources.
         * @param view_id View id to remove.
         * @return True if removed, false if not found.
         */
        auto remove_view(const QUuid& view_id) -> bool;

        /**
         * @brief Set the current view id, emit `current_view_id_changed` on success.
         * @param view_id View id to set as current.
         * @return True if view exists and was set, false otherwise.
         */
        auto set_current_view(const QUuid& view_id) -> bool;

        /**
         * @brief Returns the current view id (may be null).
         * @return QUuid Current view id.
         */
        [[nodiscard]] auto get_current_view() const -> QUuid;

        /**
         * @brief Return `LogViewContext*` for the given view id or nullptr.
         * @param view_id View id to lookup.
         * @return LogViewContext* or nullptr.
         */
        [[nodiscard]] auto get_context(const QUuid& view_id) const -> LogViewContext*;

        /**
         * @brief Return all view ids currently registered.
         * @return QVector<QUuid> of view ids.
         */
        [[nodiscard]] auto get_all_view_ids() const -> QVector<QUuid>;

        /**
         * @brief Return all entries for the given view.
         * @param view_id View id to query.
         * @return QVector<LogEntry> Entries (empty if none / view missing).
         */
        [[nodiscard]] auto get_entries(const QUuid& view_id) const -> QVector<LogEntry>;

        /**
         * @brief Return loaded absolute file paths for the given view.
         * @param view_id View id to query.
         * @return QVector<QString> File paths (empty if none / view missing).
         */
        [[nodiscard]] auto get_file_paths(const QUuid& view_id) const -> QVector<QString>;

        /**
         * @brief Replace the loaded-files list for a view and notify listeners.
         * @param view_id Target view id.
         * @param files List of LogFileInfo to set.
         */
        auto set_loaded_files(const QUuid& view_id, const QList<LogFileInfo>& files) -> void;

        /**
         * @brief Add a single loaded file to a view and notify listeners.
         * @param view_id Target view id.
         * @param file File info to add.
         */
        auto add_loaded_file(const QUuid& view_id, const LogFileInfo& file) -> void;

        /**
         * @brief Remove all entries for `file_path` from the specified view.
         *        Also removes the file from the view's loaded-file list and notifies listeners.
         * @param view_id Target view id.
         * @param file_path Absolute file path to remove.
         */
        auto remove_entries_by_file(const QUuid& view_id, const QString& file_path) -> void;

        /**
         * @brief Export a view's state including loaded files, filters, paging and sort.
         * @param view_id Target view id.
         * @param filters Coordinator used to read current filter/visibility on the view.
         * @return SessionViewState snapshot, including a suggested tab title.
         */
        [[nodiscard]] auto export_view_state(
            const QUuid& view_id, const FilterCoordinator& filters) const -> SessionViewState;

        /**
         * @brief Import a view state: ensure/create view, set loaded files, reapply filters,
         *        and paging/sort.
         * @param state Serialized view state to apply.
         * @param filters Coordinator used to apply filters.
         * @return QUuid The (ensured) view id of the imported state.
         *
         * Round-trip guarantee:
         * Applying an exported `SessionViewState` reconstructs the same logical view configuration,
         * assuming the referenced files exist and are accessible on the current machine.
         */
        auto import_view_state(const SessionViewState& state, FilterCoordinator& filters) -> QUuid;

    signals:
        /**
         * @brief Emitted when the current view id changes.
         */
        void current_view_id_changed(const QUuid& view_id);

        /**
         * @brief Emitted when a view is removed.
         */
        void view_removed(const QUuid& view_id);

        /**
         * @brief Emitted when the set of file paths for a view changes.
         * @param view_id The affected view.
         * @param file_paths Current list of file paths in the view.
         */
        void view_file_paths_changed(const QUuid& view_id, const QVector<QString>& file_paths);

    private:
        QMap<QUuid, LogViewContext*> m_contexts;
        QUuid m_current_view_id;
};
