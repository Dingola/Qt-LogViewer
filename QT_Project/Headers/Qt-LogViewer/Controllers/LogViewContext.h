#pragma once

#include <QList>
#include <QObject>
#include <QString>
#include <QUuid>
#include <QVector>

#include "Qt-LogViewer/Models/LogFileInfo.h"
#include "Qt-LogViewer/Models/LogModel.h"
#include "Qt-LogViewer/Models/LogSortFilterProxyModel.h"
#include "Qt-LogViewer/Models/PagingProxyModel.h"

/**
 * @file LogViewContext.h
 * @brief Declares the LogViewContext class that encapsulates per-view components and state.
 */

/**
 * @class LogViewContext
 * @brief Per-view container that bundles model, proxy chain, and loaded-file state.
 *
 * Responsibilities:
 * - Own and wire the per-view model chain: LogModel -> LogSortFilterProxyModel -> PagingProxyModel.
 * - Track the files loaded in this view.
 * - Provide convenience methods to append/remove entries and query file paths.
 *
 */
class LogViewContext final: public QObject
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs a LogViewContext and wires the model/proxy chain.
         * @param parent The QObject parent for ownership.
         */
        explicit LogViewContext(QObject* parent = nullptr);

        /**
         * @brief Destroys the context and its owned components.
         */
        ~LogViewContext() override;

        /**
         * @brief Returns the underlying LogModel.
         * @return Pointer to LogModel (never nullptr after construction).
         */
        [[nodiscard]] auto get_model() const -> LogModel*;

        /**
         * @brief Returns the sort/filter proxy.
         * @return Pointer to LogSortFilterProxyModel (never nullptr after construction).
         */
        [[nodiscard]] auto get_sort_proxy() const -> LogSortFilterProxyModel*;

        /**
         * @brief Returns the paging proxy.
         * @return Pointer to PagingProxyModel (never nullptr after construction).
         */
        [[nodiscard]] auto get_paging_proxy() const -> PagingProxyModel*;

        /**
         * @brief Appends log entries to the underlying model.
         * @param entries The batch of entries to append.
         */
        auto append_entries(const QVector<LogEntry>& entries) -> void;

        /**
         * @brief Removes all entries belonging to the given file path from the model.
         * @param file_path Absolute file path to remove.
         */
        auto remove_entries_by_file_path(const QString& file_path) -> void;

        /**
         * @brief Returns a copy of all entries currently in the model.
         * @return Vector of LogEntry.
         */
        [[nodiscard]] auto get_entries() const -> QVector<LogEntry>;

        /**
         * @brief Sets the list of files considered loaded for this view.
         * @param files The list to set.
         */
        auto set_loaded_files(const QList<LogFileInfo>& files) -> void;

        /**
         * @brief Adds a single file to the loaded-files list (no duplicates).
         * @param file_info The file info to add.
         */
        auto add_loaded_file(const LogFileInfo& file_info) -> void;

        /**
         * @brief Returns the loaded file infos.
         * @return List of LogFileInfo.
         */
        [[nodiscard]] auto get_loaded_files() const -> QList<LogFileInfo>;

        /**
         * @brief Returns the absolute file paths for all loaded files in this view.
         * @return Vector of absolute file paths.
         */
        [[nodiscard]] auto get_file_paths() const -> QVector<QString>;

        /**
         * @brief Clears the loaded-files list.
         */
        auto clear_loaded_files() -> void;

    private:
        LogModel* m_model;
        LogSortFilterProxyModel* m_sort_proxy;
        PagingProxyModel* m_paging_proxy;

        QList<LogFileInfo> m_loaded_files;
};
