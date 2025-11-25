#pragma once

#include <QAction>
#include <QDockWidget>
#include <QMainWindow>
#include <QModelIndex>
#include <QPlainTextEdit>
#include <QVector>

#include "Qt-LogViewer/Controllers/LogViewerController.h"
#include "Qt-LogViewer/Models/LogEntry.h"
#include "Qt-LogViewer/Models/LogFileInfo.h"
#include "Qt-LogViewer/Services/StylesheetLoader.h"
#include "Qt-LogViewer/Views/App/LogFileExplorer.h"
#include "Qt-LogViewer/Views/App/LogLevelPieChartWidget.h"
#include "Qt-LogViewer/Views/Shared/BaseMainWindow.h"
#include "Qt-LogViewer/Views/Shared/DockWidget.h"

namespace Ui
{
class MainWindow;
}

class LogViewerSettings;

/**
 * @class MainWindow
 * @brief The main application window class.
 *
 * This class represents the main window of the application, providing
 * the central user interface and handling main window events.
 */
class MainWindow: public BaseMainWindow
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs a MainWindow object.
         *
         * Initializes the main window and its user interface.
         *
         * @param settings Optional app settings.
         * @param parent The parent widget, or nullptr if this is a top-level window.
         */
        explicit MainWindow(LogViewerSettings* settings = nullptr, QWidget* parent = nullptr);

        /**
         * @brief Destroys the MainWindow object.
         *
         * Cleans up any resources used by the main window.
         */
        ~MainWindow() override;

    private:
        /**
         * @brief Sets up the log file explorer dock widget and its connections.
         */
        auto setup_log_file_explorer() -> void;

        /**
         * @brief Sets up the log level pie chart dock widget.
         */
        auto setup_log_level_pie_chart() -> void;

        /**
         * @brief Sets up pagination widget.
         */
        auto setup_pagination_widget() -> void;

        /**
         * @brief Sets up the log details dock widget.
         */
        auto setup_log_details_dock() -> void;

        /**
         * @brief Sets up the filter bar widget.
         */
        auto setup_filter_bar() -> void;

        /**
         * @brief Sets up the tab widget.
         */
        auto setup_tab_widget() -> void;

        /**
         * @brief Initializes the main menu bar and its actions.
         */
        auto initialize_menu() -> void;

        /**
         * @brief Updates the log details view when a row is selected.
         * @param current The current selected index.
         */
        auto update_log_details(const QModelIndex& current) -> void;

        /**
         * @brief Updates the pagination widget based on the current page and total pages.
         *
         * This method retrieves the current page and total pages from the proxy model,
         * ensures they are within valid ranges, and updates the pagination widget accordingly.
         */
        auto update_pagination_widget() -> void;

        /**
         * @brief Handles drag enter events to allow dropping log files.
         * @param event The drag enter event.
         */
        void dragEnterEvent(QDragEnterEvent* event) override;

        /**
         * @brief Handles drop events to load log files via drag and drop.
         * @param event The drop event.
         */
        void dropEvent(QDropEvent* event) override;

        /**
         * @brief Handles resize events to adjust the layout.
         * @param event The resize event.
         */
        void resizeEvent(QResizeEvent* event) override;

    protected:
        /**
         * @brief Handles change events to update the UI.
         * @param event The change event.
         */
        auto changeEvent(QEvent* event) -> void override;

    private slots:
        /**
         * @brief Opens log files using a file dialog and adds them into the LogFileExplorer.
         */
        void handle_open_log_file_dialog_requested();

        /**
         * @brief Shows the settings dialog for changing application settings.
         */
        void handle_show_settings_dialog_requested();

        /**
         * @brief Handles search changes in the filter bar widget.
         *
         * This method retrieves the search text, field, and regex status from the filter bar
         * widget, then updates the controller's search filter accordingly.
         */
        auto handle_search_changed() -> void;

        /**
         * @brief Handles open log file requests and creates a new tab with a LogViewWidget.
         *        Uses streaming loading to keep the UI responsive.
         * @param log_file_info The LogFileInfo to load and display.
         */
        auto handle_log_file_open_requested(const LogFileInfo& log_file_info) -> void;

        /**
         * @brief Slot to handle changes in the current view ID.
         * @param view_id The new current view ID.
         */
        auto handle_current_view_id_changed(const QUuid& view_id) -> void;

        /**
         * @brief Handles removal of a view by closing the corresponding tab.
         * @param view_id The QUuid of the removed view.
         */
        auto handle_view_removed(const QUuid& view_id) -> void;

        /**
         * @brief Handles streaming progress for a specific view.
         * @param view_id The target view.
         * @param bytes_read Bytes read so far.
         * @param total_bytes Total file size in bytes.
         */
        auto handle_loading_progress(const QUuid& view_id, qint64 bytes_read,
                                     qint64 total_bytes) -> void;

        /**
         * @brief Handles streaming completion for a file/view.
         * @param view_id The view that received the data.
         * @param file_path The file that finished streaming.
         */
        auto handle_loading_finished(const QUuid& view_id, const QString& file_path) -> void;

        /**
         * @brief Handles streaming errors.
         * @param view_id The target view.
         * @param file_path The file that errored.
         * @param message Error message.
         */
        auto handle_loading_error(const QUuid& view_id, const QString& file_path,
                                  const QString& message) -> void;

    private:
        Ui::MainWindow* ui;
        LogViewerSettings* m_log_viewer_settings = nullptr;
        LogViewerController* m_controller = nullptr;
        QAction* m_action_open_log_file = nullptr;
        QAction* m_action_quit = nullptr;
        QAction* m_action_show_log_file_explorer = nullptr;
        QAction* m_action_show_log_details = nullptr;
        QAction* m_action_show_log_level_pie_chart = nullptr;
        QAction* m_action_settings = nullptr;
        DockWidget* m_log_details_dock_widget = nullptr;
        DockWidget* m_log_file_explorer_dock_widget = nullptr;
        DockWidget* m_log_level_pie_chart_dock_widget = nullptr;
        QPlainTextEdit* m_log_details_text_edit = nullptr;
        LogFileExplorer* m_log_file_explorer = nullptr;
        LogLevelPieChartWidget* m_log_level_pie_chart_widget = nullptr;
};
