#include "Qt-LogViewer/Views/MainWindow.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QIcon>
#include <QMessageBox>
#include <QMimeData>
#include <QPlainTextEdit>
#include <QResizeEvent>
#include <QStackedWidget>
#include <QStringList>
#include <QTimer>
#include <QUrl>
#include <optional>

#include "Qt-LogViewer/Controllers/DockController.h"
#include "Qt-LogViewer/Controllers/LogViewerController.h"
#include "Qt-LogViewer/Controllers/MainMenuController.h"
#include "Qt-LogViewer/Controllers/SessionController.h"
#include "Qt-LogViewer/Models/LogFileTreeModel.h"
#include "Qt-LogViewer/Models/RecentItemsModel.h"
#include "Qt-LogViewer/Models/RecentListSchema.h"
#include "Qt-LogViewer/Presenters/LogImportTabPresenter.h"
#include "Qt-LogViewer/Presenters/LogViewPresenter.h"
#include "Qt-LogViewer/Presenters/WorkspacePresenter.h"
#include "Qt-LogViewer/Services/LogParsingProfile.h"
#include "Qt-LogViewer/Services/LogViewerSettings.h"
#include "Qt-LogViewer/Services/SessionManager.h"
#include "Qt-LogViewer/Services/SessionRepository.h"
#include "Qt-LogViewer/Views/App/Dialogs/SettingsDialog.h"
#include "Qt-LogViewer/Views/App/LogFileExplorer.h"
#include "Qt-LogViewer/Views/App/LogLevelPieChartWidget.h"
#include "Qt-LogViewer/Views/App/LogTableView.h"
#include "Qt-LogViewer/Views/App/LogViewWidget.h"
#include "Qt-LogViewer/Views/App/StartPageWidget.h"
#include "Qt-LogViewer/Views/Shared/DockWidget.h"
#include "QtWidgetsCommonLib/Services/Translator.h"
#include "QtWidgetsCommonLib/Utils/StylesheetLoader.h"
#include "ui_MainWindow.h"

namespace
{
constexpr auto k_open_log_files_text = QT_TRANSLATE_NOOP("MainWindow", "Open Log Files");
constexpr auto k_loaded_log_files_status = QT_TRANSLATE_NOOP("MainWindow", "Loaded %1 log file(s)");
constexpr auto k_untitled_session_text = QT_TRANSLATE_NOOP("MainWindow", "Untitled Session");

}  // namespace

using QtWidgetsCommonLib::AppMainWindow;

/**
 * @brief Constructs a MainWindow object.
 *
 * Initializes the main window, sets up the user interface, menu, status bar, and connects all
 * signals/slots.
 *
 * @param settings The application settings.
 * @param parent The parent widget, or nullptr if this is a top-level window.
 */
MainWindow::MainWindow(LogViewerSettings* settings, QWidget* parent)
    : AppMainWindow(settings, parent),
      m_log_viewer_settings(settings),
      m_controller(
          new LogViewerController(LogParsingProfile::create_default(
                                      QStringLiteral("{timestamp} {level} {message} {app_name}"),
                                      QStringLiteral("Qt-LogViewer default")),
                                  this)),
      m_log_level_pie_chart_widget(new LogLevelPieChartWidget(this)),
      ui(new Ui::MainWindow)
{
    qDebug() << "MainWindow constructor started";
    qInfo() << "Settings file:" << m_log_viewer_settings->fileName()
            << "| Format:" << m_log_viewer_settings->format()
            << "| Scope:" << m_log_viewer_settings->scope()
            << "| Organization:" << m_log_viewer_settings->organizationName()
            << "| Application:" << m_log_viewer_settings->applicationName();

    ui->setupUi(this);
    setContentsMargins(9, 9, 9, 9);
    setWindowIcon(QIcon(":/Resources/Icons/App/AppIcon.svg"));

    // Replace central widget with QStackedWidget to allow Start Page
    QWidget* old_central = this->takeCentralWidget();
    auto* central_stack = new QStackedWidget(this);
    setCentralWidget(central_stack);
    if (old_central != nullptr)
    {
        central_stack->addWidget(old_central);
    }

    // Initialize session manager and recent models (unified, schema-driven)
    m_session_manager = new SessionManager(new SessionRepository(this), this);
    m_session_manager->initialize_from_storage();

    const RecentListSchema files_schema = RecentListSchemas::make_recent_files_schema();
    const RecentListSchema sessions_schema = RecentListSchemas::make_recent_sessions_schema();

    m_recent_files_model = new RecentItemsModel(files_schema, this);
    m_recent_sessions_model = new RecentItemsModel(sessions_schema, this);

    // Build rows from SessionManager data and set them on the generic model
    {
        QVector<QHash<int, QVariant>> file_rows;
        const auto recent_files = m_session_manager->get_recent_log_files();
        file_rows.reserve(recent_files.size());
        for (const auto& rf: recent_files)
        {
            file_rows.push_back(RecentListSchemas::build_recent_file_row(rf));
        }
        m_recent_files_model->set_rows(std::move(file_rows));
    }
    {
        QVector<QHash<int, QVariant>> session_rows;
        const auto recent_sessions = m_session_manager->get_recent_sessions();
        session_rows.reserve(recent_sessions.size());
        for (const auto& rs: recent_sessions)
        {
            session_rows.push_back(RecentListSchemas::build_recent_session_row(rs));
        }
        m_recent_sessions_model->set_rows(std::move(session_rows));
    }

    connect(m_session_manager, &SessionManager::recent_log_files_changed, this,
            [this](const QVector<RecentLogFileRecord>& items) {
                QVector<QHash<int, QVariant>> rows;
                rows.reserve(items.size());
                for (const auto& rf: items)
                {
                    rows.push_back(RecentListSchemas::build_recent_file_row(rf));
                }
                m_recent_files_model->set_rows(std::move(rows));
                if (m_menu_controller != nullptr)
                {
                    m_menu_controller->rebuild_recent_menus();
                }
            });
    connect(m_session_manager, &SessionManager::recent_sessions_changed, this,
            [this](const QVector<RecentSessionRecord>& items) {
                QVector<QHash<int, QVariant>> rows;
                rows.reserve(items.size());
                for (const auto& rs: items)
                {
                    rows.push_back(RecentListSchemas::build_recent_session_row(rs));
                }
                m_recent_sessions_model->set_rows(std::move(rows));
                if (m_menu_controller != nullptr)
                {
                    m_menu_controller->rebuild_recent_menus();
                }
            });

    setup_log_file_explorer();
    setup_log_level_pie_chart();
    setup_pagination_widget();
    setup_log_details_dock();
    setup_filter_bar();
    setup_tab_widget();

    m_session_controller = new SessionController(
        m_session_manager, m_controller->get_log_file_tree_model(), m_controller, this);

    // Connect session controller signals
    connect(m_session_controller, &SessionController::expand_session_requested, m_log_file_explorer,
            &LogFileExplorer::expand_session);
    connect(m_session_controller, &SessionController::all_sessions_removed, this,
            &MainWindow::handle_all_sessions_removed);
    connect(m_session_controller, &SessionController::session_restore_started, this,
            [this](const QString&) { close_all_tabs(); });
    connect(m_session_controller, &SessionController::view_restored, this,
            &MainWindow::restore_view_from_state);
    connect(m_session_controller, &SessionController::session_renamed, this,
            [this](const QString&, const QString&) { m_menu_controller->rebuild_recent_menus(); });
    connect(m_session_controller, &SessionController::session_deleted, this,
            [this](const QString&) { m_menu_controller->rebuild_recent_menus(); });

    // Start page widget (stack page 1)
    auto* start_page = new StartPageWidget(central_stack);
    start_page->set_recent_files_model(m_recent_files_model);
    start_page->set_recent_sessions_model(m_recent_sessions_model);
    connect(start_page, &StartPageWidget::open_log_file_requested, this,
            &MainWindow::handle_open_log_file_dialog_requested);
    connect(start_page, &StartPageWidget::open_recent_file_requested, this,
            &MainWindow::handle_open_recent_file);
    connect(start_page, &StartPageWidget::clear_recent_files_requested, this,
            &MainWindow::handle_clear_recent_files);
    connect(start_page, &StartPageWidget::open_session_requested, this,
            &MainWindow::handle_open_session_dialog);
    connect(start_page, &StartPageWidget::open_recent_session_requested, this,
            &MainWindow::handle_open_recent_session);
    connect(start_page, &StartPageWidget::reopen_last_session_requested, this,
            &MainWindow::handle_reopen_last_session);
    connect(start_page, &StartPageWidget::delete_session_requested, this,
            &MainWindow::handle_delete_session);
    central_stack->addWidget(start_page);

    connect(m_controller, &LogViewerController::loading_progress, this,
            &MainWindow::handle_loading_progress);
    connect(m_controller, &LogViewerController::loading_finished, this,
            &MainWindow::handle_loading_finished);
    connect(m_controller, &LogViewerController::loading_error, this,
            &MainWindow::handle_loading_error);
    initialize_dock_controller();
    m_workspace_presenter = new WorkspacePresenter(
        m_controller, m_session_controller, ui->tabWidgetLog, ui->logFilterBarWidget,
        ui->paginationWidget, m_log_details_text_edit, m_log_level_pie_chart_widget, central_stack,
        m_dock_controller, this);
    m_log_import_tab_presenter = new LogImportTabPresenter(
        *m_log_viewer_settings, *m_controller->get_preview_service(),
        *m_controller->get_import_coordinator(), *ui->tabWidgetLog,
        [this](const QUuid& view_id) {
            SessionViewState empty_state;
            return create_log_view_widget_for_view(view_id, empty_state);
        },
        m_session_controller, m_workspace_presenter, this);
    connect(m_log_import_tab_presenter, &LogImportTabPresenter::status_message_requested, this,
            [this](const QString& message, int timeout_ms) {
                statusBar()->showMessage(message, timeout_ms);
            });
    initialize_menu();
    m_menu_controller->rebuild_recent_menus();

    QTimer::singleShot(0, this, [this] { this->resizeEvent(nullptr); });
    qDebug() << "MainWindow constructor finished";
}

/**
 * @brief Destroys the MainWindow object.
 *
 * Cleans up any resources used by the main window.
 */
MainWindow::~MainWindow()
{
    qDebug() << "MainWindow destructor called";
    delete ui;
}

/**
 * @brief Sets up the log file explorer dock widget and its connections.
 */
auto MainWindow::setup_log_file_explorer() -> void
{
    m_log_file_explorer = new LogFileExplorer(m_controller->get_log_file_tree_model(), this);
    m_log_file_explorer_dock_widget = new DockWidget(tr("Log File Explorer"), this);
    m_log_file_explorer_dock_widget->setContentsMargins(0, 0, 0, 0);
    m_log_file_explorer_dock_widget->setTitleBarWidget(
        DockWidget::create_dock_title_bar(m_log_file_explorer_dock_widget));
    m_log_file_explorer_dock_widget->setObjectName("logFileExplorerDockWidget");
    m_log_file_explorer_dock_widget->setWidget(m_log_file_explorer);
    addDockWidget(Qt::LeftDockWidgetArea, m_log_file_explorer_dock_widget);
    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);

    connect(m_log_file_explorer, &LogFileExplorer::open_file_requested, this,
            &MainWindow::handle_log_file_open_requested);
    connect(m_log_file_explorer, &LogFileExplorer::add_to_current_view_requested, this,
            &MainWindow::handle_add_log_file_to_current_view_requested);
    connect(
        m_log_file_explorer, &LogFileExplorer::remove_file_requested, m_controller,
        [this](const LogFileInfo& log_file_info) { m_controller->remove_log_file(log_file_info); });

    // Session actions from LogFileExplorer - delegate to SessionController
    connect(m_log_file_explorer, &LogFileExplorer::rename_session_requested, this,
            &MainWindow::handle_rename_session);
    connect(m_log_file_explorer, &LogFileExplorer::close_session_requested, this,
            &MainWindow::handle_close_session);
    connect(m_log_file_explorer, &LogFileExplorer::delete_session_requested, this,
            &MainWindow::handle_delete_session);
}

/**
 * @brief Sets up the log level pie chart dock widget.
 */
auto MainWindow::setup_log_level_pie_chart() -> void
{
    m_log_level_pie_chart_dock_widget = new DockWidget(tr("Log Level Pie Chart"), this);
    m_log_level_pie_chart_dock_widget->setContentsMargins(0, 0, 0, 0);
    m_log_level_pie_chart_dock_widget->setTitleBarWidget(
        DockWidget::create_dock_title_bar(m_log_level_pie_chart_dock_widget));
    m_log_level_pie_chart_dock_widget->setObjectName("logLevelPieChartDockWidget");
    m_log_level_pie_chart_dock_widget->setWidget(m_log_level_pie_chart_widget);
    addDockWidget(Qt::LeftDockWidgetArea, m_log_level_pie_chart_dock_widget);
    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
}

/**
 * @brief Sets up the pagination widget.
 */
auto MainWindow::setup_pagination_widget() -> void
{
    ui->paginationWidget->set_max_page_buttons(7);
}

/**
 * @brief Sets up the log details dock widget.
 */
auto MainWindow::setup_log_details_dock() -> void
{
    m_log_details_dock_widget = new DockWidget(tr("Log Details"), this);
    m_log_details_dock_widget->setContentsMargins(0, 0, 0, 0);
    m_log_details_dock_widget->setObjectName("logDetailsDockWidget");
    m_log_details_dock_widget->setTitleBarWidget(
        DockWidget::create_dock_title_bar(m_log_details_dock_widget));
    m_log_details_text_edit = new QPlainTextEdit(m_log_details_dock_widget);
    m_log_details_text_edit->setObjectName("logDetailsTextEdit");
    m_log_details_text_edit->setReadOnly(true);
    m_log_details_dock_widget->setWidget(m_log_details_text_edit);
    addDockWidget(Qt::BottomDockWidgetArea, m_log_details_dock_widget);
}

/**
 * @brief Sets up the filter bar widget.
 */
auto MainWindow::setup_filter_bar() -> void
{
    QVector<QString> available_log_levels = m_controller->get_available_log_levels({});
    ui->logFilterBarWidget->setContentsMargins(0, 0, 0, 0);
    ui->logFilterBarWidget->set_filter_widget_visible(false);
    ui->logFilterBarWidget->set_search_bar_enabled(false);
    ui->logFilterBarWidget->set_available_log_levels(available_log_levels);
}

/**
 * @brief Sets up the tab widget.
 */
auto MainWindow::setup_tab_widget() -> void
{
    ui->tabWidgetLog->setup_default_behavior();
}

/**
 * @brief Initializes the dock controller and registers the application docks.
 */
auto MainWindow::initialize_dock_controller() -> void
{
    m_dock_controller = new DockController(this, this);
    m_dock_controller->register_dock(m_log_file_explorer_dock_widget);
    m_dock_controller->register_dock(m_log_details_dock_widget);
    m_dock_controller->register_dock(m_log_level_pie_chart_dock_widget);
}

/**
 * @brief Initializes the main menu controller and its connections.
 */
auto MainWindow::initialize_menu() -> void
{
    m_menu_controller =
        new MainMenuController(ui->menubar, m_recent_files_model, m_recent_sessions_model, this);

    // Add the native toggle action of each registered dock to the Views menu
    for (QAction* toggle_action: m_dock_controller->get_toggle_actions())
    {
        m_menu_controller->add_view_action(toggle_action);
    }

    connect(m_menu_controller, &MainMenuController::open_log_file_requested, this,
            &MainWindow::handle_open_log_file_dialog_requested);
    connect(m_menu_controller, &MainMenuController::open_recent_file_requested, this,
            &MainWindow::handle_open_recent_file);
    connect(m_menu_controller, &MainMenuController::clear_recent_files_requested, this,
            &MainWindow::handle_clear_recent_files);
    connect(m_menu_controller, &MainMenuController::save_session_requested, this,
            &MainWindow::handle_save_session);
    connect(m_menu_controller, &MainMenuController::open_session_requested, this,
            &MainWindow::handle_open_session_dialog);
    connect(m_menu_controller, &MainMenuController::open_recent_session_requested, this,
            &MainWindow::handle_open_session);
    connect(m_menu_controller, &MainMenuController::reopen_last_session_requested, this,
            &MainWindow::handle_reopen_last_session);
    connect(m_menu_controller, &MainMenuController::settings_requested, this,
            &MainWindow::handle_show_settings_dialog_requested);
    connect(m_menu_controller, &MainMenuController::quit_requested, this,
            &QApplication::closeAllWindows);

    connect(m_menu_controller, &MainMenuController::about_requested, this, [this] {
        QMessageBox::about(this, tr("About %1").arg(QCoreApplication::applicationName()),
                           tr("<b>%1</b><br>"
                              "Version 1.0<br>"
                              "&copy; 2025 Adrian Helbig<br>"
                              "Built with Qt %2<br>"
                              "<a href=\"https://AdrianHelbig.de\">AdrianHelbig.de</a>")
                               .arg(QCoreApplication::applicationName(), QT_VERSION_STR));
    });
    connect(m_menu_controller, &MainMenuController::about_qt_requested, this,
            [this] { QMessageBox::aboutQt(this); });
}

/**
 * @brief Handles drag enter events to allow dropping log files.
 * @param event The drag enter event.
 */
void MainWindow::dragEnterEvent(QDragEnterEvent* event)
{
    qDebug() << "Drag enter event with URLs:" << event->mimeData()->urls();

    if (event->mimeData()->hasUrls())
    {
        event->acceptProposedAction();
    }
}

/**
 * @brief Handles drop events to load log files via drag and drop.
 * @param event The drop event.
 */
void MainWindow::dropEvent(QDropEvent* event)
{
    QStringList files;

    for (const QUrl& url: event->mimeData()->urls())
    {
        files << url.toLocalFile();
    }

    qDebug() << "Files dropped:" << files;

    const QString session_id =
        m_session_controller->ensure_current_session(tr(k_untitled_session_text));
    m_session_controller->add_files_to_current_session(files);
    m_session_controller->request_expand_session(session_id);
    m_workspace_presenter->refresh_start_page();
}

/**
 * @brief Handles resize events to adjust the layout.
 * @param event The resize event.
 */
void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);

    if (m_dock_controller != nullptr && event != nullptr)
    {
        m_dock_controller->handle_main_window_resize(event->oldSize(), event->size());
    }

    if (ui != nullptr && ui->tabWidgetLog != nullptr)
    {
        ui->tabWidgetLog->auto_resize_current_columns();
    }
}

/**
 * @brief Handles change events to update the UI.
 * @param event The change event.
 */
auto MainWindow::changeEvent(QEvent* event) -> void
{
    if (event != nullptr && event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        if (m_menu_controller != nullptr)
        {
            m_menu_controller->retranslate();
            m_menu_controller->rebuild_recent_menus();
        }
        m_log_file_explorer_dock_widget->setWindowTitle(tr("Log File Explorer"));
        m_log_details_dock_widget->setWindowTitle(tr("Log Details"));
        m_workspace_presenter->refresh_active_view();
    }

    AppMainWindow::changeEvent(event);
}

/**
 * @brief Handles show events to display the start page if needed.
 * @param event The show event.
 */
void MainWindow::showEvent(QShowEvent* event)
{
    AppMainWindow::showEvent(event);
    m_dock_controller->capture_current_sizes();
    m_workspace_presenter->refresh_start_page();
}

/**
 * @brief Handles the close event to save window settings.
 * @param event The close event.
 */
auto MainWindow::closeEvent(QCloseEvent* event) -> void
{
    if (m_session_controller->has_current_session())
    {
        m_session_controller->save_current_session();
    }
    else
    {
        // Restore suspended docks before AppMainWindow persists the window layout.
        m_dock_controller->set_docks_suspended(false);
    }

    AppMainWindow::closeEvent(event);
}

/**
 * @brief Opens log files using a file dialog and adds them into the LogFileExplorer.
 */
void MainWindow::handle_open_log_file_dialog_requested()
{
    qDebug() << "Opening log file dialog";
    QStringList files = QFileDialog::getOpenFileNames(this, tr(k_open_log_files_text), QString(),
                                                      tr("Log Files (*.log *.txt);;All Files (*)"));
    qDebug() << "Files selected:" << files;

    if (!files.isEmpty())
    {
        const QString session_id =
            m_session_controller->ensure_current_session(tr(k_untitled_session_text));

        QVector<QString> valid_files = validate_file_paths(files);

        if (!valid_files.isEmpty())
        {
            m_session_controller->add_files_to_current_session(valid_files);

            for (const QString& file: valid_files)
            {
                m_session_controller->add_recent_log_file(LogFileInfo(file));
            }

            m_session_controller->request_expand_session(session_id);
            m_session_controller->save_current_session();
            m_menu_controller->rebuild_recent_menus();
        }

        m_workspace_presenter->refresh_start_page();
    }
}

/**
 * @brief Validates a list of file paths and returns only valid ones.
 * @param files The list of file paths to validate.
 * @return Vector of valid file paths.
 */
auto MainWindow::validate_file_paths(const QStringList& files) -> QVector<QString>
{
    QVector<QString> valid_files;
    QStringList invalid_files;

    for (const QString& path: files)
    {
        QFileInfo fi(path);
        const bool ok = fi.exists() && fi.isFile() && fi.isReadable();
        if (ok)
        {
            valid_files.append(fi.absoluteFilePath());
        }
        else
        {
            invalid_files.append(path);
        }
    }

    if (!invalid_files.isEmpty())
    {
        QMessageBox::warning(this, tr("Invalid Files"),
                             tr("The following files could not be opened or are invalid:\n%1")
                                 .arg(invalid_files.join(QStringLiteral("\n"))));
    }

    return valid_files;
}

/**
 * @brief Shows the settings dialog for changing application settings.
 */
void MainWindow::handle_show_settings_dialog_requested()
{
    SettingsDialog dialog(m_log_viewer_settings, this);
    dialog.setWindowTitle("SettingsDialog");
    dialog.resize(440, 300);
    dialog.set_available_themes(AppMainWindow::get_stylesheet_loader()->get_available_themes());
    dialog.set_available_language_names(
        AppMainWindow::get_translator()->get_available_language_names());

    dialog.exec();
}

/**
 * @brief Open selected session by id from menu or start page.
 * @param session_id Session identifier.
 */
auto MainWindow::handle_open_session(const QString& session_id) -> void
{
    if (!session_id.isEmpty())
    {
        const std::optional<SessionState> state = m_session_controller->load_session(session_id);
        if (state.has_value())
        {
            restore_session(*state);
        }
        else
        {
            QMessageBox::warning(this, tr("Open Session"),
                                 tr("Selected session could not be loaded:\n%1").arg(session_id));
        }
    }
}

/**
 * @brief Open selected recent session by id from menu or start page.
 * @param session_id Session identifier.
 */
auto MainWindow::handle_open_recent_session(const QString& session_id) -> void
{
    handle_open_session(session_id);
}

/**
 * @brief Clear recent files list via session controller.
 */
auto MainWindow::handle_clear_recent_files() -> void
{
    m_session_controller->clear_recent_log_files();
}

/**
 * @brief Save current session snapshot via session controller.
 */
auto MainWindow::handle_save_session() -> void
{
    m_session_controller->save_current_session();
    m_menu_controller->rebuild_recent_menus();
}

/**
 * @brief Delete session by id via session controller.
 * @param session_id Session identifier.
 */
auto MainWindow::handle_delete_session(const QString& session_id) -> void
{
    m_session_controller->delete_session(session_id);

    if (m_session_controller->get_session_count() == 0)
    {
        close_all_tabs();
    }

    m_workspace_presenter->refresh_start_page();
}

/**
 * @brief Opens a session file via file dialog.
 */
auto MainWindow::handle_open_session_dialog() -> void
{
    const QString session_file = QFileDialog::getOpenFileName(
        this, tr("Open Session"), QString(), tr("Session Files (*.json);;All Files (*)"));

    if (!session_file.isEmpty())
    {
        QFileInfo fi(session_file);
        const bool ok = fi.exists() && fi.isFile() && fi.isReadable();

        if (ok)
        {
            const QString session_id = fi.completeBaseName();
            handle_open_session(session_id);
        }
        else
        {
            QMessageBox::warning(this, tr("Open Session"),
                                 tr("Failed to open session file:\n%1").arg(session_file));
        }
    }
}

/**
 * @brief Reopen last session id if available.
 */
auto MainWindow::handle_reopen_last_session() -> void
{
    const QString id = m_session_controller->get_last_session_id();
    if (!id.isEmpty())
    {
        handle_open_session(id);
    }
}

/**
 * @brief Restores a session from typed state.
 * @param state The loaded session state.
 */
auto MainWindow::restore_session(const SessionState& state) -> void
{
    if (!state.id.isEmpty())
    {
        m_workspace_presenter->set_session_active(true);

        QString profile_error;
        const QVector<LogParsingProfile> available_profiles =
            m_log_viewer_settings->get_log_parsing_profiles(&profile_error);

        if (!profile_error.isEmpty())
        {
            qWarning() << "Could not load parsing profiles while restoring session:"
                       << profile_error;
        }

        const bool restored = m_session_controller->restore_session(state, available_profiles);

        if (restored)
        {
            m_session_controller->request_expand_session(state.id);
            m_menu_controller->rebuild_recent_menus();
            m_workspace_presenter->refresh_active_view();
            m_workspace_presenter->refresh_start_page();
        }
        else
        {
            QMessageBox::warning(this, tr("Open Session"),
                                 tr("Session data could not be restored."));
        }
    }
    else
    {
        QMessageBox::warning(this, tr("Open Session"), tr("Session data is invalid or empty."));
    }
}

/**
 * @brief Restores a single view from typed session state.
 * @param view_id Restored view identifier.
 * @param state The view state.
 */
auto MainWindow::restore_view_from_state(const QUuid& view_id,
                                         const SessionViewState& state) -> void
{
    LogViewWidget* log_view_widget = create_log_view_widget_for_view(view_id, state);

    const QVector<QString> view_paths = m_controller->get_view_file_paths(view_id);
    const QString tab_title = state.tab_title.isEmpty() && !view_paths.isEmpty()
                                  ? QFileInfo(view_paths.first()).fileName()
                                  : state.tab_title;

    const int tab_index = ui->tabWidgetLog->add_log_view_tab(log_view_widget, tab_title, true);

    if (tab_index < 0)
    {
        qWarning() << "Failed to add restored view tab:" << view_id;
    }
}

/**
 * @brief Creates a LogViewWidget and attaches its per-view presenter.
 * @param view_id The view ID.
 * @param state The session view state.
 * @return Pointer to the created LogViewWidget.
 */
auto MainWindow::create_log_view_widget_for_view(const QUuid& view_id,
                                                 const SessionViewState& state) -> LogViewWidget*
{
    auto* log_view_widget = new LogViewWidget(ui->tabWidgetLog);
    auto* presenter =
        new LogViewPresenter(m_controller, log_view_widget, view_id, state, log_view_widget);
    m_workspace_presenter->bind_log_view_presenter(presenter);

    return log_view_widget;
}

/**
 * @brief Closes all tabs in the tab widget.
 */
auto MainWindow::close_all_tabs() -> void
{
    ui->tabWidgetLog->close_all_view_tabs();
}

/**
 * @brief Open selected recent file from menu or start page.
 * @param file_path Absolute file path.
 */
auto MainWindow::handle_open_recent_file(const QString& file_path) -> void
{
    if (!file_path.isEmpty())
    {
        const QString session_id =
            m_session_controller->ensure_current_session(tr(k_untitled_session_text));

        m_session_controller->add_file_to_session(session_id, file_path);
        m_session_controller->add_recent_log_file(LogFileInfo(file_path));
        m_session_controller->request_expand_session(session_id);
        m_session_controller->save_current_session();

        m_menu_controller->rebuild_recent_menus();
        m_workspace_presenter->refresh_start_page();
    }
}

/**
 * @brief Handles open log file requests and creates a new tab with a LogViewWidget.
 * @param log_file_info The LogFileInfo to load and display.
 */
auto MainWindow::handle_log_file_open_requested(const LogFileInfo& log_file_info) -> void
{
    m_log_import_tab_presenter->show_import_tab(log_file_info);
}

/**
 * @brief Handles requests to add a log file to the current view.
 * @param log_file_info The LogFileInfo to add.
 */
auto MainWindow::handle_add_log_file_to_current_view_requested(const LogFileInfo& log_file_info)
    -> void
{
    const QUuid current_view = m_controller->get_current_view();

    if (!current_view.isNull())
    {
        m_log_import_tab_presenter->show_import_tab(log_file_info, current_view);
    }
    else
    {
        handle_log_file_open_requested(log_file_info);
    }
}

/**
 * @brief Handles streaming progress for a specific view.
 * @param view_id The target view.
 * @param bytes_read Bytes read so far.
 * @param total_bytes Total file size in bytes.
 */
auto MainWindow::handle_loading_progress(const QUuid& view_id, qint64 bytes_read,
                                         qint64 total_bytes) -> void
{
    Q_UNUSED(view_id);
    int percent = 0;

    if (total_bytes > 0)
    {
        percent = static_cast<int>(
            (static_cast<double>(bytes_read) * 100.0) / static_cast<double>(total_bytes) + 0.5);
    }

    statusBar()->showMessage(
        tr("Loading... %1% (%2 / %3 bytes)").arg(percent).arg(bytes_read).arg(total_bytes));
}

/**
 * @brief Handles the completion of log file loading.
 * @param view_id The QUuid of the view that finished loading.
 * @param file_path The path of the loaded log file.
 */
auto MainWindow::handle_loading_finished(const QUuid& view_id, const QString& file_path) -> void
{
    Q_UNUSED(view_id);
    QFileInfo info(file_path);
    statusBar()->showMessage(tr("Loaded %1 (%2 bytes)").arg(info.fileName()).arg(info.size()),
                             4000);
}

/**
 * @brief Handles streaming errors.
 * @param view_id The QUuid of the view that encountered an error.
 * @param file_path The path of the log file that failed to load.
 * @param message The error message.
 */
auto MainWindow::handle_loading_error(const QUuid& view_id, const QString& file_path,
                                      const QString& message) -> void
{
    Q_UNUSED(view_id);
    statusBar()->clearMessage();

    QMessageBox::critical(this, tr("Load Error"),
                          tr("Failed to load file:\n%1\n\n%2").arg(file_path, message));
}

/**
 * @brief Handles rename session requests from the LogFileExplorer.
 * @param session_id The session identifier.
 * @param new_name The new session name.
 */
auto MainWindow::handle_rename_session(const QString& session_id, const QString& new_name) -> void
{
    m_session_controller->rename_session(session_id, new_name);
}

/**
 * @brief Handles the signal when all sessions are removed from the tree model.
 */
auto MainWindow::handle_all_sessions_removed() -> void
{
    close_all_tabs();

    // Clear all views to prevent stale data
    m_session_controller->clear_all_views();

    m_workspace_presenter->reset_presentation();
    m_workspace_presenter->refresh_pagination();
    m_workspace_presenter->refresh_start_page();
}

/**
 * @brief Handles close session requests from the LogFileExplorer.
 * @param session_id The session identifier.
 */
auto MainWindow::handle_close_session(const QString& session_id) -> void
{
    close_all_tabs();

    m_session_controller->close_session(session_id);

    if (m_session_controller->get_session_count() == 0)
    {
        m_workspace_presenter->reset_presentation();
        m_workspace_presenter->refresh_pagination();
    }

    m_workspace_presenter->refresh_start_page();
}
