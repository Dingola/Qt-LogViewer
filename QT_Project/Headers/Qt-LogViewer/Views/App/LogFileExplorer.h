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
 * @brief Widget for displaying and managing grouped log files in a tree view.
 *
 * This widget shows log files grouped by application name using LogFileTreeModel.
 * Supports adding and removing files dynamically.
 */
class LogFileExplorer: public QWidget
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs a LogFileExplorer widget.
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

    private:
        /**
         * @brief Sets up connections for signals and slots.
         *
         * Connects the tree view's selection changed signal to the file_selected signal.
         */
        auto setup_connections() -> void;

        /**
         * @brief Initializes the context menu for the tree view.
         */
        auto init_context_menu() -> void;

        /**
         * @brief Handles context menu requests on the tree view.
         * @param pos The position where the menu should appear.
         */
        auto show_context_menu(const QPoint& pos) -> void;

        /**
         * @brief Tries to extract a LogFileInfo for the given index if the item type matches.
         * @param index The model index to inspect.
         * @param expected_type The expected LogFileTreeItem::Type (e.g., File, Group).
         * @param out_info Output parameter that receives the LogFileInfo on success.
         * @return True if the index is valid, the item type matches, and out_info was set; false
         * otherwise.
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
         * @brief Emitted when the user requests to add a log file to the current view via the
         * context menu.
         * @param log_file_info The LogFileInfo to add to the current view.
         */
        void add_to_current_view_requested(const LogFileInfo& log_file_info);

        /**
         * @brief Emitted when the user requests to remove a log file via the context menu.
         * @param log_file_info The LogFileInfo to remove.
         */
        void remove_file_requested(const LogFileInfo& log_file_info);

    private:
        Ui::LogFileExplorer* ui;
        LogFileTreeModel* m_model = nullptr;
        QMenu* m_context_menu = nullptr;
};
