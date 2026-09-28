#pragma once

#include <QMainWindow>
#include <QString>
#include <QUuid>
#include <QVector>

#include "QtWidgetsCommonLib/Widgets/AppMainWindow.h"

// Forward declarations for Qt types used as pointers/references
class QDockWidget;
class QPlainTextEdit;
class QResizeEvent;
class QDragEnterEvent;
class QDropEvent;
class QCloseEvent;
class QEvent;

// Forward declarations for project types used as pointers/references
namespace Ui
{
class MainWindow;
}

class LogViewerSettings;
class LogParsingProfile;
class FileCatalogController;
class FilterCoordinator;
class LiveTailingCoordinator;
class LogHistoryService;
class LogImportCoordinator;
class LogPageCoordinator;
class LogPreviewService;
class LogQueryController;
class ViewLifecycleCoordinator;
class ViewRegistry;
class DockController;
class MainMenuController;
class SessionController;
class LogImportTabPresenter;
class WorkspacePresenter;
class LogFileInfo;
class RecentItemsModel;
class SessionManager;
class RecentItemsAdapter;
class LogFileExplorer;
class LogLevelPieChartWidget;
class LogViewWidget;
class StartPageWidget;
class DockWidget;
struct SessionViewState;
struct SessionState;

/**
 * @class MainWindow
 * @brief The main application window class.
 *
 * This class represents the main window of the application, providing
 * the central user interface and handling main window events.
 */
class MainWindow: public QtWidgetsCommonLib::AppMainWindow
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs a MainWindow object.
         *
         * Initializes the main window and its user interface.
         *
         * @param settings Application settings owned by the composition root.
         * @param catalog File catalog used by the explorer.
         * @param views Runtime view registry.
         * @param filters Per-view filter state.
         * @param history Persistent log history.
         * @param pages Per-view pagination state.
         * @param queries Query operations.
         * @param imports Log import workflow.
         * @param lifecycle View and file cleanup workflow.
         * @param live_tailing Live-tailing workflow.
         * @param preview Log preview service used by import tabs.
         * @param session_manager Session state service owned by the composition root.
         * @param recent_items_adapter Adapter supplying synchronized recent-item models.
         * @param session_controller Session workflow controller owned by the composition root.
         * @param parent The parent widget, or nullptr if this is a top-level window.
         */
        explicit MainWindow(LogViewerSettings& settings, FileCatalogController& catalog,
                            ViewRegistry& views, FilterCoordinator& filters,
                            LogHistoryService& history, LogPageCoordinator& pages,
                            LogQueryController& queries, LogImportCoordinator& imports,
                            ViewLifecycleCoordinator& lifecycle,
                            LiveTailingCoordinator& live_tailing, LogPreviewService& preview,
                            SessionManager& session_manager,
                            RecentItemsAdapter& recent_items_adapter,
                            SessionController& session_controller, QWidget* parent = nullptr);

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
         * @brief Initializes the dock controller and registers the application docks.
         */
        auto initialize_dock_controller() -> void;

        /**
         * @brief Initializes the main menu controller and its connections.
         *
         * Connects menu requests to the existing window handlers and adds registered dock actions.
         */
        auto initialize_menu() -> void;

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
         * @brief Restores a single view from typed session state.
         * @param view_id Restored view identifier.
         * @param state The view state.
         */
        auto restore_view_from_state(const QUuid& view_id, const SessionViewState& state) -> void;

        /**
         * @brief Creates a LogViewWidget and attaches its per-view presenter.
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
         * @brief Restores a session from typed state.
         * @param state The loaded session state.
         */
        auto restore_session(const SessionState& state) -> void;

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
        FileCatalogController* m_catalog{nullptr};
        ViewRegistry* m_views{nullptr};
        FilterCoordinator* m_filters{nullptr};
        LogHistoryService* m_history{nullptr};
        LogPageCoordinator* m_pages{nullptr};
        LogQueryController* m_queries{nullptr};
        LogImportCoordinator* m_imports{nullptr};
        ViewLifecycleCoordinator* m_lifecycle{nullptr};
        LiveTailingCoordinator* m_live_tailing{nullptr};
        LogPreviewService* m_preview{nullptr};
        DockController* m_dock_controller = nullptr;
        MainMenuController* m_menu_controller = nullptr;
        LogImportTabPresenter* m_log_import_tab_presenter = nullptr;
        WorkspacePresenter* m_workspace_presenter = nullptr;

        // Session-related
        SessionManager* m_session_manager = nullptr;
        RecentItemsAdapter* m_recent_items_adapter = nullptr;

        // Unified, schema-driven recent models
        RecentItemsModel* m_recent_files_model = nullptr;
        RecentItemsModel* m_recent_sessions_model = nullptr;

        SessionController* m_session_controller{nullptr};

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
};
