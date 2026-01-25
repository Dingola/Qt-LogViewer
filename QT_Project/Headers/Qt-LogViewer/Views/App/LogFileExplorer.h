#pragma once

#include <QMenu>
#include <QPoint>
#include <QTreeView>
#include <QWidget>
#include <functional>

#include "Qt-LogViewer/Models/LogFileTreeItem.h"

namespace Ui
{
class LogFileExplorer;
}

class LogFileTreeModel;
class LogFileInfo;

/**
 * @class LogFileExplorer
 * @brief Widget for displaying and managing sessions and grouped log files in a tree view.
 *
 * This widget shows sessions with log files grouped by application name using LogFileTreeModel.
 * Supports adding and removing sessions, files, and dynamic updates.
 */
class LogFileExplorer: public QWidget
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs a LogFileExplorer widget.
         * @param model The LogFileTreeModel to use, or nullptr to create a new one.
         * @param parent The parent widget, or nullptr.
         */
        explicit LogFileExplorer(LogFileTreeModel* model = nullptr, QWidget* parent = nullptr);

        /**
         * @brief Destroys the LogFileExplorer widget.
         */
        ~LogFileExplorer() override;

        /**
         * @brief Sets the model for the tree view.
         * @param model Pointer to the LogFileTreeModel to set.
         */
        auto set_model(LogFileTreeModel* model) -> void;

        /**
         * @brief Returns the underlying tree view.
         * @return Pointer to the QTreeView.
         */
        [[nodiscard]] auto tree_view() const -> QTreeView*;

        /**
         * @brief Returns the underlying model.
         * @return Pointer to the LogFileTreeModel.
         */
        [[nodiscard]] auto model() const -> LogFileTreeModel*;

        /**
         * @brief Sets the log files to display in the tree view.
         * @param log_file_infos The list of LogFileInfo objects.
         */
        auto set_log_files(const QList<LogFileInfo>& log_file_infos) -> void;

        /**
         * @brief Expands the tree to show a specific session.
         * @param session_id The session identifier to expand.
         */
        auto expand_session(const QString& session_id) -> void;

    private:
        /**
         * @brief Sets up connections for signals and slots.
         */
        auto setup_connections() -> void;

        /**
         * @brief Initializes the context menus for different item types.
         */
        auto init_context_menu() -> void;

        /**
         * @brief Handles context menu requests on the tree view.
         * @param pos The position where the menu should appear.
         */
        auto show_context_menu(const QPoint& pos) -> void;

        /**
         * @brief Gets the item type at the given index.
         * @param index The model index.
         * @return The item type, or Group if invalid.
         */
        [[nodiscard]] auto get_item_type(const QModelIndex& index) const -> LogFileTreeItem::Type;

        /**
         * @brief Tries to extract a LogFileInfo for the given index if the item type matches.
         * @param index The model index to inspect.
         * @param expected_type The expected LogFileTreeItem::Type.
         * @param out_info Output parameter that receives the LogFileInfo on success.
         * @return True if successful; false otherwise.
         */
        [[nodiscard]] auto try_get_info_if_type(const QModelIndex& index,
                                                LogFileTreeItem::Type expected_type,
                                                LogFileInfo& out_info) const -> bool;

        /**
         * @brief Dispatches a callable for the current index if the item type matches.
         * @param expected_type The expected LogFileTreeItem::Type.
         * @param fn The callable to invoke with LogFileInfo when type matches.
         * @return True if dispatched; false otherwise.
         */
        [[nodiscard]] auto dispatch_current_if_type(
            LogFileTreeItem::Type expected_type,
            const std::function<void(const LogFileInfo&)>& fn) const -> bool;

    protected:
        /**
         * @brief Handles change events to update the UI.
         * @param event The change event.
         */
        auto changeEvent(QEvent* event) -> void override;

    signals:
        /**
         * @brief Emitted when a log file is selected in the tree view.
         * @param log_file_info The selected LogFileInfo.
         */
        void file_selected(const LogFileInfo& log_file_info);

        /**
         * @brief Emitted when the user requests to open a log file via the context menu.
         * @param log_file_info The LogFileInfo to open.
         */
        void open_file_requested(const LogFileInfo& log_file_info);

        /**
         * @brief Emitted when the user requests to add a log file to the current view.
         * @param log_file_info The LogFileInfo to add to the current view.
         */
        void add_to_current_view_requested(const LogFileInfo& log_file_info);

        /**
         * @brief Emitted when the user requests to remove a log file via the context menu.
         * @param log_file_info The LogFileInfo to remove.
         */
        void remove_file_requested(const LogFileInfo& log_file_info);

        /**
         * @brief Emitted when a session is selected.
         * @param session_id The session identifier.
         */
        void session_selected(const QString& session_id);

        /**
         * @brief Emitted when the user requests to rename a session.
         * @param session_id The session identifier.
         * @param new_name The new session name.
         */
        void rename_session_requested(const QString& session_id, const QString& new_name);

        /**
         * @brief Emitted when the user requests to close (remove from view) a session.
         * @param session_id The session identifier.
         */
        void close_session_requested(const QString& session_id);

        /**
         * @brief Emitted when the user requests to delete a session permanently.
         * @param session_id The session identifier.
         */
        void delete_session_requested(const QString& session_id);

    private:
        Ui::LogFileExplorer* ui;
        LogFileTreeModel* m_model = nullptr;
        QMenu* m_file_context_menu = nullptr;
        QMenu* m_session_context_menu = nullptr;
        QMenu* m_group_context_menu = nullptr;
};
