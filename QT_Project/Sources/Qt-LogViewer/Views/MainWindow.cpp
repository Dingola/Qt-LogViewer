#include "Qt-LogViewer/Views/MainWindow.h"

#include <QAction>
#include <QCoreApplication>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QIcon>
#include <QItemSelectionModel>
#include <QJsonArray>
#include <QJsonObject>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QSet>
#include <QStackedWidget>
#include <QStringList>
#include <QTimer>
#include <QUrl>

#include "Qt-LogViewer/Models/LogModel.h"
#include "Qt-LogViewer/Models/LogSortFilterProxyModel.h"
#include "Qt-LogViewer/Services/LogViewerSettings.h"
#include "Qt-LogViewer/Services/SessionRepository.h"
#include "Qt-LogViewer/Services/Translator.h"
#include "Qt-LogViewer/Views/App/Dialogs/SettingsDialog.h"
#include "Qt-LogViewer/Views/App/StartPageWidget.h"
#include "ui_MainWindow.h"

namespace
{
constexpr auto k_open_log_files_text = QT_TRANSLATE_NOOP("MainWindow", "Open Log Files");
constexpr auto k_open_log_file_text = QT_TRANSLATE_NOOP("MainWindow", "Open Log File...");
constexpr auto k_file_menu_text = QT_TRANSLATE_NOOP("MainWindow", "&File");
constexpr auto k_recent_files_text = QT_TRANSLATE_NOOP("MainWindow", "Recent Files");
constexpr auto k_recent_sessions_text = QT_TRANSLATE_NOOP("MainWindow", "Recent Sessions");
constexpr auto k_save_session_text = QT_TRANSLATE_NOOP("MainWindow", "Save Session...");
constexpr auto k_open_session_text = QT_TRANSLATE_NOOP("MainWindow", "Open Session...");
constexpr auto k_reopen_last_session_text = QT_TRANSLATE_NOOP("MainWindow", "Reopen Last Session");
constexpr auto k_loaded_log_files_status = QT_TRANSLATE_NOOP("MainWindow", "Loaded %1 log file(s)");
constexpr auto k_quit_text = QT_TRANSLATE_NOOP("MainWindow", "&Quit");
constexpr auto k_views_menu_text = QT_TRANSLATE_NOOP("MainWindow", "&Views");
constexpr auto k_show_log_file_explorer_text =
    QT_TRANSLATE_NOOP("MainWindow", "Show Log File Explorer");
constexpr auto k_show_log_details_text = QT_TRANSLATE_NOOP("MainWindow", "Show Log Details");
constexpr auto k_untitled_session_text = QT_TRANSLATE_NOOP("MainWindow", "Untitled Session");
}  // namespace

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
    : BaseMainWindow(settings, parent),
      m_log_viewer_settings(settings),
      m_controller(new LogViewerController("{timestamp} {level} {message} {app_name}", this)),
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

    // Initialize session manager and recent models
    m_session_manager = new SessionManager(new SessionRepository(this), this);
    m_session_manager->initialize_from_storage();

    m_recent_files_model = new RecentLogFilesModel(this);
    m_recent_sessions_model = new RecentSessionsModel(this);
    m_recent_files_model->set_items(m_session_manager->get_recent_log_files());
    m_recent_sessions_model->set_items(m_session_manager->get_recent_sessions());

    connect(m_session_manager, &SessionManager::recent_log_files_changed, this,
            [this](const QVector<RecentLogFileRecord>& items) {
                m_recent_files_model->set_items(items);
                rebuild_recent_menus();
            });
    connect(m_session_manager, &SessionManager::recent_sessions_changed, this,
            [this](const QVector<RecentSessionRecord>& items) {
                m_recent_sessions_model->set_items(items);
                rebuild_recent_menus();
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
    connect(m_session_controller, &SessionController::current_session_changed, this,
            [this](const QString&) { show_start_page_if_needed(); });
    connect(m_session_controller, &SessionController::session_renamed, this,
            [this](const QString&, const QString&) { rebuild_recent_menus(); });
    connect(m_session_controller, &SessionController::session_deleted, this,
            [this](const QString&) { rebuild_recent_menus(); });

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

    m_controller->set_current_view(m_controller->load_log_files({}));
    connect(m_controller, &LogViewerController::current_view_id_changed, this,
            &MainWindow::handle_current_view_id_changed);
    connect(m_controller, &LogViewerController::view_removed, this,
            &MainWindow::handle_view_removed);
    connect(m_controller, &LogViewerController::loading_progress, this,
            &MainWindow::handle_loading_progress);
    connect(m_controller, &LogViewerController::loading_finished, this,
            &MainWindow::handle_loading_finished);
    connect(m_controller, &LogViewerController::loading_error, this,
            &MainWindow::handle_loading_error);
    connect(m_controller, &LogViewerController::view_file_paths_changed, this,
            [this](const QUuid& view_id, const QVector<QString>& file_paths) {
                const int tab_count = ui->tabWidgetLog->count();
                for (int i = 0; i < tab_count; ++i)
                {
                    auto* view = qobject_cast<LogViewWidget*>(ui->tabWidgetLog->widget(i));
                    if (view != nullptr && view->get_view_id() == view_id)
                    {
                        view->set_view_file_paths(file_paths);
                    }
                }
            });

    initialize_menu();
    rebuild_recent_menus();

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
    connect(m_log_file_explorer, &LogFileExplorer::remove_file_requested, m_controller,
            [this](const LogFileInfo& log_file_info) {
                m_controller->remove_log_file(log_file_info);
                update_pagination_widget();
            });

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
 * @brief Sets up the log table view and pagination widget.
 */
auto MainWindow::setup_pagination_widget() -> void
{
    ui->paginationWidget->set_max_page_buttons(7);

    connect(ui->paginationWidget, &PaginationWidget::page_changed, this,
            [this](int page) { m_controller->get_paging_proxy()->set_current_page(page); });
    connect(ui->paginationWidget, &PaginationWidget::items_per_page_changed, this,
            [this](int items_per_page) {
                auto* proxy = m_controller->get_paging_proxy();

                if (proxy != nullptr)
                {
                    proxy->set_page_size(items_per_page);
                    proxy->set_current_page(1);
                    update_pagination_widget();
                }
            });
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
    ui->logFilterBarWidget->set_available_log_levels(available_log_levels);
    connect(ui->logFilterBarWidget, &LogFilterBarWidget::app_filter_changed, this,
            [this](const QString& app_name) {
                m_controller->set_app_name_filter(app_name);
                update_pagination_widget();
            });
    connect(ui->logFilterBarWidget, &LogFilterBarWidget::log_level_filter_changed, this,
            [this](const QSet<QString>& log_levels) {
                qDebug() << "Level filter set to:" << log_levels;
                m_controller->set_log_level_filters(log_levels);
                update_pagination_widget();
            });

    // React only to search_requested; avoids double-calling when live search is on.
    connect(ui->logFilterBarWidget, &LogFilterBarWidget::search_requested, this,
            &MainWindow::handle_search_changed);
    connect(ui->logFilterBarWidget, &LogFilterBarWidget::search_field_changed, this,
            &MainWindow::handle_search_changed);
    connect(ui->logFilterBarWidget, &LogFilterBarWidget::regex_toggled, this,
            &MainWindow::handle_search_changed);
}

/**
 * @brief Sets up the tab widget.
 */
auto MainWindow::setup_tab_widget() -> void
{
    ui->tabWidgetLog->setTabsClosable(true);
    ui->tabWidgetLog->setElideMode(Qt::ElideRight);
    ui->tabWidgetLog->tabBar()->setMovable(true);

    connect(ui->tabWidgetLog, &QTabWidget::currentChanged, this, [this](int index) {
        auto* log_view_widget = qobject_cast<LogViewWidget*>(ui->tabWidgetLog->widget(index));
        if (log_view_widget != nullptr)
        {
            QUuid view_id = log_view_widget->get_view_id();
            m_controller->set_current_view(view_id);
            QVector<QString> file_paths = m_controller->get_view_file_paths(view_id);
            log_view_widget->set_view_file_paths(file_paths);
            update_pagination_widget();
        }
    });
    connect(ui->tabWidgetLog, &TabWidget::about_to_close_tab, this, [this](int index) {
        auto* log_view_widget = qobject_cast<LogViewWidget*>(ui->tabWidgetLog->widget(index));
        if (log_view_widget != nullptr)
        {
            QUuid view_id = log_view_widget->get_view_id();
            m_controller->remove_view(view_id);
        }
    });
    connect(ui->tabWidgetLog, &TabWidget::close_tab_requested, this, [this](int index) {
        Q_UNUSED(index);
        if (ui->tabWidgetLog->count() == 0)
        {
            m_log_level_pie_chart_widget->set_log_level_counts({});
            update_pagination_widget();
        }
    });
}

/**
 * @brief Initializes the main menu bar and its actions.
 *
 * Adds File menu entries and views menu. Also creates Recent Files / Recent Sessions
 * submenus and session-related actions (Save/Open/Reopen Last Session).
 */
auto MainWindow::initialize_menu() -> void
{
    // File menu
    m_file_menu = new QMenu(tr(k_file_menu_text), this);

    // Action to open log files
    m_action_open_log_file = new QAction(tr(k_open_log_file_text), this);
    m_action_open_log_file->setShortcut(QKeySequence::Open);
    m_file_menu->addAction(m_action_open_log_file);
    ui->menubar->addMenu(m_file_menu);

    connect(m_action_open_log_file, &QAction::triggered, this,
            &MainWindow::handle_open_log_file_dialog_requested);

    // Recent submenus
    m_recent_files_menu = new QMenu(tr(k_recent_files_text), this);
    m_recent_sessions_menu = new QMenu(tr(k_recent_sessions_text), this);
    m_file_menu->addMenu(m_recent_files_menu);
    m_file_menu->addMenu(m_recent_sessions_menu);

    // Session actions
    m_action_save_session = new QAction(tr(k_save_session_text), this);
    m_action_open_session = new QAction(tr(k_open_session_text), this);
    m_action_reopen_last_session = new QAction(tr(k_reopen_last_session_text), this);
    m_file_menu->addSeparator();
    m_file_menu->addAction(m_action_save_session);
    m_file_menu->addAction(m_action_open_session);
    m_file_menu->addAction(m_action_reopen_last_session);

    connect(m_action_save_session, &QAction::triggered, this, [this]() { handle_save_session(); });
    connect(m_action_open_session, &QAction::triggered, this,
            [this]() { handle_open_session_dialog(); });
    connect(m_action_reopen_last_session, &QAction::triggered, this,
            [this]() { handle_reopen_last_session(); });

    // Separator before the quit action
    m_file_menu->addSeparator();
    // Action to quit the application
    m_action_quit = new QAction(tr(k_quit_text), this);
#ifdef Q_OS_WIN
    m_action_quit->setShortcut(QKeySequence(QStringLiteral("Ctrl+Q")));
#else
    m_action_quit->setShortcut(QKeySequence::Quit);
#endif
    m_file_menu->addAction(m_action_quit);
    ui->menubar->addMenu(m_file_menu);

    connect(m_action_quit, &QAction::triggered, this, &QApplication::closeAllWindows);

    // Views menu
    auto views_menu = new QMenu(tr(k_views_menu_text), this);

    m_action_show_log_file_explorer = new QAction(tr(k_show_log_file_explorer_text), this);
    m_action_show_log_file_explorer->setCheckable(true);
    views_menu->addAction(m_action_show_log_file_explorer);

    m_action_show_log_details = new QAction(tr(k_show_log_details_text), this);
    m_action_show_log_details->setCheckable(true);
    views_menu->addAction(m_action_show_log_details);

    m_action_show_log_level_pie_chart = new QAction(tr("Show Log Level Pie Chart"), this);
    m_action_show_log_level_pie_chart->setCheckable(true);
    views_menu->addAction(m_action_show_log_level_pie_chart);

    ui->menubar->addMenu(views_menu);

    // Update docks on toggle and, if a session is active, cache the new dock layout
    auto cache_dock_state_if_session = [this]() -> void {
        if (m_session_controller != nullptr && m_session_controller->has_current_session())
        {
            m_last_session_dock_state = saveState();
        }
    };

    connect(m_action_show_log_file_explorer, &QAction::toggled, this,
            [this, cache_dock_state_if_session](bool checked) {
                m_log_file_explorer_dock_widget->setVisible(checked);
                cache_dock_state_if_session();
            });
    connect(m_log_file_explorer_dock_widget, &DockWidget::closed, this, [this]() {
        m_action_show_log_file_explorer->setChecked(false);
        if (m_session_controller != nullptr && m_session_controller->has_current_session())
        {
            m_last_session_dock_state = saveState();
        }
    });

    connect(m_action_show_log_details, &QAction::toggled, this,
            [this, cache_dock_state_if_session](bool checked) {
                m_log_details_dock_widget->setVisible(checked);
                cache_dock_state_if_session();
            });
    connect(m_log_details_dock_widget, &DockWidget::closed, this, [this]() {
        m_action_show_log_details->setChecked(false);
        if (m_session_controller != nullptr && m_session_controller->has_current_session())
        {
            m_last_session_dock_state = saveState();
        }
    });

    connect(m_action_show_log_level_pie_chart, &QAction::toggled, this,
            [this, cache_dock_state_if_session](bool checked) {
                m_log_level_pie_chart_dock_widget->setVisible(checked);
                cache_dock_state_if_session();
            });
    connect(m_log_level_pie_chart_dock_widget, &DockWidget::closed, this, [this]() {
        m_action_show_log_level_pie_chart->setChecked(false);
        if (m_session_controller != nullptr && m_session_controller->has_current_session())
        {
            m_last_session_dock_state = saveState();
        }
    });

    // Settings menu
    auto settings_menu = new QMenu(tr("&Settings"), this);
    m_action_settings = new QAction(tr("Settings..."), this);
    m_action_settings->setShortcut(QKeySequence(QStringLiteral("Ctrl+,")));
    settings_menu->addAction(m_action_settings);
    ui->menubar->addMenu(settings_menu);
    connect(m_action_settings, &QAction::triggered, this,
            &MainWindow::handle_show_settings_dialog_requested);

    // Help menu
    auto help_menu = new QMenu(tr("&Help"), this);
    auto about_action = new QAction(tr("About %1").arg(QCoreApplication::applicationName()), this);
    auto about_qt_action = new QAction(tr("About Qt"), this);
    help_menu->addAction(about_action);
    help_menu->addAction(about_qt_action);
    ui->menubar->addMenu(help_menu);

    connect(about_action, &QAction::triggered, this, [this] {
        QMessageBox::about(this, tr("About %1").arg(QCoreApplication::applicationName()),
                           tr("<b>%1</b><br>"
                              "Version 1.0<br>"
                              "&copy; 2025 Adrian Helbig<br>"
                              "Built with Qt %2<br>"
                              "<a href=\"https://AdrianHelbig.de\">AdrianHelbig.de</a>")
                               .arg(QCoreApplication::applicationName(), QT_VERSION_STR));
    });
    connect(about_qt_action, &QAction::triggered, this, [this] { QMessageBox::aboutQt(this); });

    // Sync action checkmarks with current dock visibility
    // (these may be overridden later by show_start_page_if_needed based on session presence)
    if (m_log_file_explorer_dock_widget != nullptr && m_action_show_log_file_explorer != nullptr)
    {
        const bool prev = m_action_show_log_file_explorer->blockSignals(true);
        m_action_show_log_file_explorer->setChecked(m_log_file_explorer_dock_widget->isVisible());
        m_action_show_log_file_explorer->blockSignals(prev);
    }
    if (m_log_details_dock_widget != nullptr && m_action_show_log_details != nullptr)
    {
        const bool prev = m_action_show_log_details->blockSignals(true);
        m_action_show_log_details->setChecked(m_log_details_dock_widget->isVisible());
        m_action_show_log_details->blockSignals(prev);
    }
    if (m_log_level_pie_chart_dock_widget != nullptr &&
        m_action_show_log_level_pie_chart != nullptr)
    {
        const bool prev = m_action_show_log_level_pie_chart->blockSignals(true);
        m_action_show_log_level_pie_chart->setChecked(
            m_log_level_pie_chart_dock_widget->isVisible());
        m_action_show_log_level_pie_chart->blockSignals(prev);
    }
}

/**
 * @brief Rebuilds the Recent Files and Recent Sessions submenus from models.
 *
 * Idempotent; clears and repopulates actions on each call.
 */
auto MainWindow::rebuild_recent_menus() -> void
{
    if (m_recent_files_menu != nullptr)
    {
        m_recent_files_menu->clear();
        for (int row = 0; row < m_recent_files_model->rowCount(); ++row)
        {
            const QModelIndex idx = m_recent_files_model->index(row, RecentLogFilesModel::FileName);
            const QString title = m_recent_files_model->data(idx, Qt::DisplayRole).toString();
            const QString path =
                m_recent_files_model->data(idx, RecentLogFilesModel::FilePathRole).toString();
            QAction* act = m_recent_files_menu->addAction(title);
            connect(act, &QAction::triggered, this,
                    [this, path]() { handle_open_recent_file(path); });
        }
        m_recent_files_menu->addSeparator();
        QAction* clear_act = m_recent_files_menu->addAction(tr("Clear Recent Files"));
        connect(clear_act, &QAction::triggered, this, [this]() { handle_clear_recent_files(); });
    }

    if (m_recent_sessions_menu != nullptr)
    {
        m_recent_sessions_menu->clear();
        for (int row = 0; row < m_recent_sessions_model->rowCount(); ++row)
        {
            const QModelIndex idx = m_recent_sessions_model->index(row, RecentSessionsModel::Name);
            const QString title = m_recent_sessions_model->data(idx, Qt::DisplayRole).toString();
            const int IdRole = Qt::UserRole + 3;
            const QString id = m_recent_sessions_model->data(idx, IdRole).toString();
            QAction* act = m_recent_sessions_menu->addAction(title);
            connect(act, &QAction::triggered, this, [this, id]() { handle_open_session(id); });
        }
    }
}

/**
 * @brief Shows the start page if there is no current session.
 */
auto MainWindow::show_start_page_if_needed() -> void
{
    const bool has_session = m_session_controller->has_current_session();

    auto* central_stack = qobject_cast<QStackedWidget*>(this->centralWidget());
    if (central_stack != nullptr)
    {
        central_stack->setCurrentIndex(has_session ? 0 : 1);
    }

    if (!has_session)
    {
        // Capture current (restored) dock state ONCE when transitioning to StartPage,
        // so we can later restore it and also persist it if the app closes while on StartPage.
        if (m_last_session_dock_state.isEmpty())
        {
            m_last_session_dock_state = saveState();
        }

        // Transient UI for StartPage: hide docks and present actions as unchecked and disabled.
        if (m_log_file_explorer_dock_widget != nullptr)
        {
            m_log_file_explorer_dock_widget->setVisible(false);
        }
        if (m_log_details_dock_widget != nullptr)
        {
            m_log_details_dock_widget->setVisible(false);
        }
        if (m_log_level_pie_chart_dock_widget != nullptr)
        {
            m_log_level_pie_chart_dock_widget->setVisible(false);
        }

        if (m_action_show_log_file_explorer != nullptr)
        {
            const bool prev = m_action_show_log_file_explorer->blockSignals(true);
            m_action_show_log_file_explorer->setChecked(false);
            m_action_show_log_file_explorer->blockSignals(prev);
            m_action_show_log_file_explorer->setEnabled(false);
        }
        if (m_action_show_log_details != nullptr)
        {
            const bool prev = m_action_show_log_details->blockSignals(true);
            m_action_show_log_details->setChecked(false);
            m_action_show_log_details->blockSignals(prev);
            m_action_show_log_details->setEnabled(false);
        }
        if (m_action_show_log_level_pie_chart != nullptr)
        {
            const bool prev = m_action_show_log_level_pie_chart->blockSignals(true);
            m_action_show_log_level_pie_chart->setChecked(false);
            m_action_show_log_level_pie_chart->blockSignals(prev);
            m_action_show_log_level_pie_chart->setEnabled(false);
        }
    }
    else
    {
        // Back to a session: re-enable actions and restore the last-session dock layout.
        if (!m_last_session_dock_state.isEmpty())
        {
            restoreState(m_last_session_dock_state);
        }

        if (m_action_show_log_file_explorer != nullptr)
        {
            m_action_show_log_file_explorer->setEnabled(true);
        }
        if (m_action_show_log_details != nullptr)
        {
            m_action_show_log_details->setEnabled(true);
        }
        if (m_action_show_log_level_pie_chart != nullptr)
        {
            m_action_show_log_level_pie_chart->setEnabled(true);
        }

        // Sync action checkmarks with the restored dock visibility without emitting toggles.
        if (m_log_file_explorer_dock_widget != nullptr &&
            m_action_show_log_file_explorer != nullptr)
        {
            const bool prev = m_action_show_log_file_explorer->blockSignals(true);
            m_action_show_log_file_explorer->setChecked(
                m_log_file_explorer_dock_widget->isVisible());
            m_action_show_log_file_explorer->blockSignals(prev);
        }
        if (m_log_details_dock_widget != nullptr && m_action_show_log_details != nullptr)
        {
            const bool prev = m_action_show_log_details->blockSignals(true);
            m_action_show_log_details->setChecked(m_log_details_dock_widget->isVisible());
            m_action_show_log_details->blockSignals(prev);
        }
        if (m_log_level_pie_chart_dock_widget != nullptr &&
            m_action_show_log_level_pie_chart != nullptr)
        {
            const bool prev = m_action_show_log_level_pie_chart->blockSignals(true);
            m_action_show_log_level_pie_chart->setChecked(
                m_log_level_pie_chart_dock_widget->isVisible());
            m_action_show_log_level_pie_chart->blockSignals(prev);
        }
    }
}

/**
 * @brief Updates the log details view when a row is selected.
 * @param current The current selected index.
 */
auto MainWindow::update_log_details(const QModelIndex& current) -> void
{
    QString details;

    if (current.isValid())
    {
        auto* paging_proxy = m_controller->get_paging_proxy();
        auto* sort_filter_proxy = m_controller->get_sort_filter_proxy();
        auto* model = m_controller->get_log_model();

        QModelIndex sort_index = paging_proxy->mapToSource(current);
        QModelIndex source_index = sort_filter_proxy->mapToSource(sort_index);

        if (source_index.isValid())
        {
            LogEntry entry = model->get_entry(source_index.row());
            details = QString("Timestamp: %1\nLevel: %2\nApp: %3\nMessage: %4")
                          .arg(entry.get_timestamp().toString("yyyy-MM-dd HH:mm:ss"))
                          .arg(entry.get_level())
                          .arg(entry.get_app_name())
                          .arg(entry.get_message());
        }
    }

    m_log_details_text_edit->setPlainText(details);
    qDebug() << "Log details updated for row:" << current.row();
}

/**
 * @brief Updates the pagination widget based on the current page and total pages.
 *
 * This method retrieves the current page and total pages from the proxy model,
 * ensures they are within valid ranges, and updates the pagination widget accordingly.
 */
auto MainWindow::update_pagination_widget() -> void
{
    int total_pages = 0;
    int current_page = 0;
    auto* proxy = m_controller->get_paging_proxy();

    if (proxy != nullptr)
    {
        total_pages = proxy->get_total_pages();
        current_page = proxy->get_current_page();
    }

    ui->paginationWidget->set_pagination(current_page, total_pages);
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
    show_start_page_if_needed();
}

/**
 * @brief Handles resize events to adjust the layout.
 * @param event The resize event.
 */
void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);

    if (ui != nullptr && ui->tabWidgetLog != nullptr)
    {
        const int tab_count = ui->tabWidgetLog->count();
        if (tab_count > 0)
        {
            QWidget* current = ui->tabWidgetLog->currentWidget();
            auto* log_view_widget = qobject_cast<LogViewWidget*>(current);

            if (log_view_widget != nullptr)
            {
                log_view_widget->auto_resize_columns();
            }
        }
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
        ui->menubar->clear();
        initialize_menu();
        rebuild_recent_menus();
        m_log_file_explorer_dock_widget->setWindowTitle(tr("Log File Explorer"));
        m_log_details_dock_widget->setWindowTitle(tr("Log Details"));
        ui->logFilterBarWidget->set_app_names(m_controller->get_app_names());
        update_pagination_widget();
    }

    BaseMainWindow::changeEvent(event);
}

/**
 * @brief Handles show events to display the start page if needed.
 * @param event The show event.
 */
void MainWindow::showEvent(QShowEvent* event)
{
    BaseMainWindow::showEvent(event);
    show_start_page_if_needed();
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
        // If we are on StartPage, restore the last-session dock state just before saving window
        // settings, so BaseMainWindow persists the session layout instead of the StartPage-off
        // layout.
        if (!m_last_session_dock_state.isEmpty())
        {
            restoreState(m_last_session_dock_state);
        }
    }

    BaseMainWindow::closeEvent(event);
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
            rebuild_recent_menus();
        }

        show_start_page_if_needed();
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
    dialog.set_available_themes(BaseMainWindow::get_stylesheet_loader()->get_available_themes());
    dialog.set_available_language_names(
        BaseMainWindow::get_translator()->get_available_language_names());

    dialog.exec();
}

/**
 * @brief Handles search changes in the filter bar widget.
 *
 * This method retrieves the search text, field, and regex status from the filter bar widget,
 * then updates the controller's search filter accordingly.
 */
auto MainWindow::handle_search_changed() -> void
{
    QString search_text = ui->logFilterBarWidget->get_search_text();
    QString field = ui->logFilterBarWidget->get_search_field();
    bool use_regex = ui->logFilterBarWidget->get_use_regex();
    qDebug() << "Search filter:" << search_text << "Field:" << field << "Regex:" << use_regex;
    m_controller->set_search_filter(search_text, field, use_regex);
    update_pagination_widget();
}

/**
 * @brief Slot to handle changes in the current view ID.
 * @param view_id The new current view ID.
 */
auto MainWindow::handle_current_view_id_changed(const QUuid& view_id) -> void
{
    QSet<QString> app_names = m_controller->get_app_names(view_id);
    ui->logFilterBarWidget->set_app_names(app_names);
    QString app_name_filter = m_controller->get_app_name_filter(view_id);
    ui->logFilterBarWidget->set_current_app_name_filter(app_name_filter);
    QVector<QString> available_log_levels = m_controller->get_available_log_levels(view_id);
    ui->logFilterBarWidget->set_available_log_levels(available_log_levels);
    QSet<QString> log_level_filters = m_controller->get_log_level_filters(view_id);
    ui->logFilterBarWidget->set_log_levels(log_level_filters);

    QMap<QString, int> level_counts = m_controller->get_log_level_counts(view_id);
    m_log_level_pie_chart_widget->set_log_level_counts(level_counts);
    ui->logFilterBarWidget->set_log_level_counts(level_counts);

    auto* log_view_widget = qobject_cast<LogViewWidget*>(ui->tabWidgetLog->currentWidget());

    if (log_view_widget != nullptr)
    {
        log_view_widget->set_app_names(app_names);
        log_view_widget->set_current_app_name_filter(app_name_filter);
        log_view_widget->set_available_log_levels(available_log_levels);
        log_view_widget->set_log_levels(log_level_filters);
        log_view_widget->set_log_level_counts(level_counts);
        log_view_widget->auto_resize_columns();

        QVector<QString> file_paths = m_controller->get_view_file_paths(view_id);
        log_view_widget->set_view_file_paths(file_paths);
    }

    update_pagination_widget();
}

/**
 * @brief Handles removal of a view by closing the corresponding tab.
 * @param view_id The QUuid of the removed view.
 */
auto MainWindow::handle_view_removed(const QUuid& view_id) -> void
{
    bool tab_removed = false;
    int tab_count = ui->tabWidgetLog->count();

    for (int i = 0; i < tab_count && !tab_removed; ++i)
    {
        auto* log_view_widget = qobject_cast<LogViewWidget*>(ui->tabWidgetLog->widget(i));

        if (log_view_widget != nullptr)
        {
            QUuid tab_view_id = log_view_widget->get_view_id();

            if (tab_view_id == view_id)
            {
                ui->tabWidgetLog->removeTab(i);
                tab_removed = true;
            }
        }
    }

    ui->logFilterBarWidget->set_app_names({});
    ui->logFilterBarWidget->set_log_levels({});
    update_pagination_widget();
}

/**
 * @brief Open selected session by id from menu or start page.
 * @param session_id Session identifier.
 */
auto MainWindow::handle_open_session(const QString& session_id) -> void
{
    if (!session_id.isEmpty())
    {
        const QJsonObject obj = m_session_controller->load_session(session_id);
        if (!obj.isEmpty())
        {
            restore_session_from_json(session_id, obj);
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
    rebuild_recent_menus();
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

    show_start_page_if_needed();
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
 * @brief Restores a session from JSON data.
 * @param session_id The session identifier.
 * @param obj The JSON object containing session data.
 */
auto MainWindow::restore_session_from_json(const QString& session_id,
                                           const QJsonObject& obj) -> void
{
    if (!session_id.isEmpty() && !obj.isEmpty())
    {
        auto* central_stack = qobject_cast<QStackedWidget*>(this->centralWidget());
        if (central_stack != nullptr)
        {
            central_stack->setCurrentIndex(0);
        }

        close_all_tabs();

        // First, restore explorer files (files that were only in the tree, not in views)
        const QJsonArray explorer_files_array =
            obj.value(QStringLiteral("explorer_files")).toArray();
        for (const auto& f: explorer_files_array)
        {
            const QJsonObject fobj = f.toObject();
            const QString file_path = fobj.value(QStringLiteral("file_path")).toString();
            const QString app_name = fobj.value(QStringLiteral("app_name")).toString();

            if (!file_path.isEmpty())
            {
                auto* tree_model = m_controller->get_log_file_tree_model();
                if (tree_model != nullptr)
                {
                    tree_model->add_log_file(session_id, LogFileInfo(file_path, app_name));
                }
            }
        }

        // Then restore views (tabs)
        const QJsonArray views_array = obj.value(QStringLiteral("views")).toArray();
        for (const auto& v: views_array)
        {
            const QJsonObject view_obj = v.toObject();
            restore_view_from_json(session_id, view_obj);
        }

        m_session_controller->request_expand_session(session_id);
        rebuild_recent_menus();
        update_pagination_widget();
        show_start_page_if_needed();
    }
    else
    {
        QMessageBox::warning(this, tr("Open Session"), tr("Session data is invalid or empty."));
    }
}

/**
 * @brief Restores a single view from JSON.
 * @param session_id The session identifier.
 * @param view_obj The view JSON object.
 */
auto MainWindow::restore_view_from_json(const QString& session_id,
                                        const QJsonObject& view_obj) -> void
{
    SessionViewState state = parse_view_state_from_json(view_obj);

    const QUuid view_id = m_controller->import_view_state_for_session(session_id, state);

    auto* log_view_widget = create_log_view_widget_for_view(view_id, state);

    const QVector<QString> view_paths = m_controller->get_view_file_paths(view_id);
    log_view_widget->set_view_file_paths(view_paths);

    const QString tab_title = state.tab_title.isEmpty() && !view_paths.isEmpty()
                                  ? QFileInfo(view_paths.first()).fileName()
                                  : state.tab_title;
    const int tab_index = ui->tabWidgetLog->addTab(log_view_widget, tab_title);
    ui->tabWidgetLog->setCurrentIndex(tab_index);

    log_view_widget->auto_resize_columns();
}

/**
 * @brief Parses a SessionViewState from a JSON object.
 * @param view_obj The view JSON object.
 * @return The parsed SessionViewState.
 */
auto MainWindow::parse_view_state_from_json(const QJsonObject& view_obj) -> SessionViewState
{
    SessionViewState state;

    const QString vid_str = view_obj.value(QStringLiteral("id")).toString();
    state.id = QUuid::fromString(QLatin1String("{") + vid_str + QLatin1String("}"));
    state.tab_title = view_obj.value(QStringLiteral("tab_title")).toString();

    const QJsonArray files_arr = view_obj.value(QStringLiteral("loaded_files")).toArray();
    for (const auto& f: files_arr)
    {
        const QJsonObject fobj = f.toObject();
        const QString fp = fobj.value(QStringLiteral("file_path")).toString();
        const QString an = fobj.value(QStringLiteral("app_name")).toString();
        if (!fp.isEmpty())
        {
            state.loaded_files.append(LogFileInfo(fp, an));
        }
    }

    const QJsonObject filters_obj = view_obj.value(QStringLiteral("filters")).toObject();
    state.filters.app_name = filters_obj.value(QStringLiteral("app_name")).toString();
    state.filters.search_text = filters_obj.value(QStringLiteral("search_text")).toString();
    state.filters.search_field = filters_obj.value(QStringLiteral("search_field")).toString();
    state.filters.use_regex = filters_obj.value(QStringLiteral("use_regex")).toBool();
    state.filters.show_only_file = filters_obj.value(QStringLiteral("show_only_file")).toString();

    const QJsonArray levels_arr = filters_obj.value(QStringLiteral("log_levels")).toArray();
    for (const auto& lv: levels_arr)
    {
        state.filters.log_levels.insert(lv.toString());
    }
    const QJsonArray hidden_arr = filters_obj.value(QStringLiteral("hidden_files")).toArray();
    for (const auto& hf: hidden_arr)
    {
        state.filters.hidden_files.insert(hf.toString());
    }

    state.page_size = static_cast<qsizetype>(view_obj.value(QStringLiteral("page_size")).toInt());
    state.current_page = static_cast<int>(view_obj.value(QStringLiteral("current_page")).toInt());
    state.sort_column = static_cast<int>(view_obj.value(QStringLiteral("sort_column")).toInt());
    const QString sort_order =
        view_obj.value(QStringLiteral("sort_order")).toString(QStringLiteral("asc"));
    state.sort_order = (sort_order.compare(QStringLiteral("desc"), Qt::CaseInsensitive) == 0)
                           ? Qt::DescendingOrder
                           : Qt::AscendingOrder;

    return state;
}

/**
 * @brief Creates a LogViewWidget for a view and connects its signals.
 * @param view_id The view ID.
 * @param state The session view state.
 * @return Pointer to the created LogViewWidget.
 */
auto MainWindow::create_log_view_widget_for_view(const QUuid& view_id,
                                                 const SessionViewState& state) -> LogViewWidget*
{
    auto* log_view_widget = new LogViewWidget(ui->tabWidgetLog);
    log_view_widget->set_view_id(view_id);

    auto* paging_proxy = m_controller->get_paging_proxy(view_id);
    log_view_widget->set_model(paging_proxy);

    const QSet<QString> app_names = m_controller->get_app_names(view_id);
    log_view_widget->set_app_names(app_names);
    log_view_widget->set_current_app_name_filter(state.filters.app_name);
    log_view_widget->set_available_log_levels(m_controller->get_available_log_levels(view_id));
    log_view_widget->set_log_levels(state.filters.log_levels);

    const QMap<QString, int> level_counts = m_controller->get_log_level_counts(view_id);
    log_view_widget->set_log_level_counts(level_counts);

    connect(log_view_widget, &LogViewWidget::current_row_changed, this,
            &MainWindow::update_log_details);
    connect(log_view_widget, &LogViewWidget::app_filter_changed, this, [this](const QString& app) {
        m_controller->set_app_name_filter(app);
        update_pagination_widget();
    });
    connect(log_view_widget, &LogViewWidget::log_level_filter_changed, this,
            [this](const QSet<QString>& levels) {
                m_controller->set_log_level_filters(levels);
                update_pagination_widget();
            });
    connect(log_view_widget, &LogViewWidget::toggle_visibility_requested, this,
            [this, view_id](const QString& file_path) {
                m_controller->toggle_file_visibility(view_id, file_path);
                update_pagination_widget();
            });
    connect(log_view_widget, &LogViewWidget::show_only_file_requested, this,
            [this, view_id](const QString& file_path) {
                m_controller->set_show_only_file(view_id, file_path);
                update_pagination_widget();
            });
    connect(log_view_widget, &LogViewWidget::remove_file_requested, this,
            [this, view_id](const QString& file_path) {
                m_controller->remove_log_file(view_id, file_path);
                update_pagination_widget();
            });

    return log_view_widget;
}

/**
 * @brief Closes all tabs in the tab widget.
 */
auto MainWindow::close_all_tabs() -> void
{
    while (ui->tabWidgetLog->count() > 0)
    {
        ui->tabWidgetLog->removeTab(0);
    }
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

        rebuild_recent_menus();
        show_start_page_if_needed();
    }
}

/**
 * @brief Handles open log file requests and creates a new tab with a LogViewWidget.
 * @param log_file_info The LogFileInfo to load and display.
 */
auto MainWindow::handle_log_file_open_requested(const LogFileInfo& log_file_info) -> void
{
    const QString session_id =
        m_session_controller->ensure_current_session(tr(k_untitled_session_text));

    auto view_id = m_controller->load_log_file_async(log_file_info.get_file_path(), 1000);
    m_controller->set_current_view(view_id);

    SessionViewState empty_state;
    auto* log_view_widget = create_log_view_widget_for_view(view_id, empty_state);

    QVector<QString> file_paths = m_controller->get_view_file_paths(view_id);
    log_view_widget->set_view_file_paths(file_paths);

    QString tab_title = log_file_info.get_file_name();
    int tab_index = ui->tabWidgetLog->addTab(log_view_widget, tab_title);
    ui->tabWidgetLog->setCurrentIndex(tab_index);
    log_view_widget->auto_resize_columns();

    m_session_controller->request_expand_session(session_id);

    update_pagination_widget();
    show_start_page_if_needed();
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
        const bool enqueued =
            m_controller->load_log_file_async(current_view, log_file_info.get_file_path(), 1000);

        if (enqueued)
        {
            statusBar()->showMessage(
                tr("Queued file for current view: %1").arg(log_file_info.get_file_name()), 3000);
        }
        else
        {
            statusBar()->showMessage(
                tr("File already present in current view: %1").arg(log_file_info.get_file_name()),
                3000);
        }
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
    bool is_current = (view_id == m_controller->get_current_view());
    int percent = 0;

    if (total_bytes > 0)
    {
        percent = static_cast<int>(
            (static_cast<double>(bytes_read) * 100.0) / static_cast<double>(total_bytes) + 0.5);
    }

    statusBar()->showMessage(
        tr("Loading... %1% (%2 / %3 bytes)").arg(percent).arg(bytes_read).arg(total_bytes));

    if (is_current)
    {
        update_pagination_widget();
    }
}

/**
 * @brief Handles the completion of log file loading.
 * @param view_id The QUuid of the view that finished loading.
 * @param file_path The path of the loaded log file.
 */
auto MainWindow::handle_loading_finished(const QUuid& view_id, const QString& file_path) -> void
{
    const bool is_current = (view_id == m_controller->get_current_view());
    QFileInfo info(file_path);
    statusBar()->showMessage(tr("Loaded %1 (%2 bytes)").arg(info.fileName()).arg(info.size()),
                             4000);

    if (is_current)
    {
        handle_current_view_id_changed(view_id);
        update_pagination_widget();
    }
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

    // Reset UI state
    ui->logFilterBarWidget->set_app_names({});
    ui->logFilterBarWidget->set_log_levels({});
    m_log_level_pie_chart_widget->set_log_level_counts({});
    update_pagination_widget();

    show_start_page_if_needed();
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
        // Reset UI state
        ui->logFilterBarWidget->set_app_names({});
        ui->logFilterBarWidget->set_log_levels({});
        m_log_level_pie_chart_widget->set_log_level_counts({});
        update_pagination_widget();
    }

    show_start_page_if_needed();
}
