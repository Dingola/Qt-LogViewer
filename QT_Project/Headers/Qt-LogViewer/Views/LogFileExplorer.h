#pragma once

#include <QTreeView>
#include <QWidget>

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
         * @param files The list of LogFileInfo objects.
         */
        auto set_log_files(const QList<LogFileInfo>& files) -> void;

    private:
        /**
         * @brief Sets up connections for signals and slots.
         *
         * Connects the tree view's selection changed signal to the logFileSelected signal.
         */
        auto setup_connections() -> void;

    public slots:
        /**
         * @brief Adds a single log file to the tree view.
         * @param file The LogFileInfo to add.
         */
        auto add_log_file(const LogFileInfo& file) -> void;

        /**
         * @brief Removes a single log file from the tree view.
         * @param file The LogFileInfo to remove.
         */
        auto remove_log_file(const LogFileInfo& file) -> void;

    signals:
        /**
         * @brief Emitted when a log file is selected in the tree view.
         * @param file The selected LogFileInfo.
         */
        void logFileSelected(const LogFileInfo& file);

    private:
        Ui::LogFileExplorer* ui;
        LogFileTreeModel* m_model = nullptr;
};
