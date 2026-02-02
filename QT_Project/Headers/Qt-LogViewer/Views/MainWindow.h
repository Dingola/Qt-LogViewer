#pragma once

#include <QAction>
#include <QDockWidget>
#include <QMainWindow>
#include <QModelIndex>
#include <QPlainTextEdit>
#include <QVector>

#include "Qt-LogViewer/Controllers/LogViewerController.h"
#include "Qt-LogViewer/Controllers/SessionController.h"
#include "Qt-LogViewer/Models/LogEntry.h"
#include "Qt-LogViewer/Models/LogFileInfo.h"
#include "Qt-LogViewer/Models/RecentItemsModel.h"
#include "Qt-LogViewer/Models/RecentListSchema.h"
#include "Qt-LogViewer/Services/SessionManager.h"
#include "Qt-LogViewer/Services/StylesheetLoader.h"
#include "Qt-LogViewer/Views/App/LogFileExplorer.h"
#include "Qt-LogViewer/Views/App/LogLevelPieChartWidget.h"
#include "Qt-LogViewer/Views/App/LogViewWidget.h"
#include "Qt-LogViewer/Views/App/StartPageWidget.h"
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
         *
         * Adds File menu entries and views menu. Also creates Recent Files / Recent Sessions
         * submenus and session-related actions (Save/Open/Reopen Last Session).
         */
        auto initialize_menu() -> void;

        /**
         * @brief Rebuilds the Recent Files and Recent Sessions submenus from models.
         *
         * Idempotent; clears and repopulates actions on each call.
         */
        auto rebuild_recent_menus() -> void;

        /**
         * @brief Shows the start page if there is no current session.
         *
         * Hides certain dock widgets and disables related actions when no session is active.
         */
        auto show_start_page_if_needed() -> void;

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

        /**
         * @brief Validates a list of file paths and returns only valid ones.
         * @param files The list of file paths to validate.
         * @return Vector of valid file paths.
         */
        [[nodiscard]] auto validate_file_paths(const QStringList& files) -> QVector<QString>;

        /**
         * @brief Restores a single view from JSON.
         * @param session_id The session identifier.
         * @param view_obj The view JSON object.
         */
        auto restore_view_from_json(const QString& session_id, const QJsonObject& view_obj) -> void;

        /**
         * @brief Parses a SessionViewState from a JSON object.
         * @param view_obj The view JSON object.
         * @return The parsed SessionViewState.
         */
        [[nodiscard]] auto parse_view_state_from_json(const QJsonObject& view_obj)
            -> SessionViewState;

        /**
         * @brief Creates a LogViewWidget for a view and connects its signals.
         * @param view_id The view ID.
         * @param state The session view state.
         * @return Pointer to the created LogViewWidget.
         */
        auto create_log_view_widget_for_view(const QUuid& view_id,
                                             const SessionViewState& state) -> LogViewWidget*;

        /**
         * @brief Closes all tabs in the tab widget.
         */
        auto close_all_tabs() -> void;

    protected:
        /**
         * @brief Handles change events to update the UI.
         * @param event The change event.
         */
        auto changeEvent(QEvent* event) -> void override;

        /**
         * @brief Handles the show event to apply the current theme.
         *
         * This method is called when the main window is shown. It applies the current theme if it
         * has not been applied yet.
         *
         * @param event The show event.
         */
        void showEvent(QShowEvent* event) override;

        /**
         * @brief Handles the close event to save window settings.
         *
         * This method is called when the main window is closed. It saves the current window
         * geometry, state, and window state to preferences.
         *
         * @param event The close event.
         */
        auto closeEvent(QCloseEvent* event) -> void override;

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
         * @brief Handles requests to add a log file to the current view.
         * @param log_file_info The LogFileInfo to add.
         */
        auto handle_add_log_file_to_current_view_requested(const LogFileInfo& log_file_info)
            -> void;

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

        /**
         * @brief Open selected recent file from menu or start page.
         * @param file_path Absolute file path.
         */
        auto handle_open_recent_file(const QString& file_path) -> void;

        /**
         * @brief Open selected recent session by id from menu or start page.
         * @param session_id Session identifier.
         */
        auto handle_open_session(const QString& session_id) -> void;

        /**
         * @brief Open selected recent session by id from menu or start page.
         * @param session_id Session identifier.
         */
        auto handle_open_recent_session(const QString& session_id) -> void;

        /**
         * @brief Clear recent files list via session manager.
         */
        auto handle_clear_recent_files() -> void;

        /**
         * @brief Save current session snapshot (single-view example).
         */
        auto handle_save_session() -> void;

        /**
         * @brief Delete session by id via session manager.
         * @param session_id Session identifier.
         */
        auto handle_delete_session(const QString& session_id) -> void;

        /**
         * @brief Opens a session file via file dialog.
         */
        auto handle_open_session_dialog() -> void;

        /**
         * @brief Reopen last session id if available.
         */
        auto handle_reopen_last_session() -> void;

        /**
         * @brief Restores a session from JSON data.
         * @param session_id The session identifier.
         * @param obj The JSON object containing session data.
         */
        auto restore_session_from_json(const QString& session_id, const QJsonObject& obj) -> void;

        /**
         * @brief Handles rename session requests from the LogFileExplorer.
         * @param session_id The session identifier.
         * @param new_name The new session name.
         */
        auto handle_rename_session(const QString& session_id, const QString& new_name) -> void;

        /**
         * @brief Handles the signal when all sessions are removed from the tree model.
         */
        auto handle_all_sessions_removed() -> void;

        /**
         * @brief Handles close session requests from the LogFileExplorer (removes from view only).
         * @param session_id The session identifier.
         */
        auto handle_close_session(const QString& session_id) -> void;

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

        // Session-related
        SessionManager* m_session_manager = nullptr;

        // Unified, schema-driven recent models
        RecentItemsModel* m_recent_files_model = nullptr;
        RecentItemsModel* m_recent_sessions_model = nullptr;

        SessionController* m_session_controller{nullptr};

        // File menu submenus
        QMenu* m_file_menu = nullptr;
        QMenu* m_recent_files_menu = nullptr;
        QMenu* m_recent_sessions_menu = nullptr;
        QAction* m_action_save_session = nullptr;
        QAction* m_action_open_session = nullptr;
        QAction* m_action_reopen_last_session = nullptr;

        // Docks
        DockWidget* m_log_details_dock_widget = nullptr;
        DockWidget* m_log_file_explorer_dock_widget = nullptr;
        DockWidget* m_log_level_pie_chart_dock_widget = nullptr;

        // Views
        QPlainTextEdit* m_log_details_text_edit = nullptr;
        LogFileExplorer* m_log_file_explorer = nullptr;
        LogLevelPieChartWidget* m_log_level_pie_chart_widget = nullptr;

        // Start page (tab area filler)
        StartPageWidget* m_start_page_widget = nullptr;

        // Stores the last known dock layout/state while a session is active.
        QByteArray m_last_session_dock_state;
};
