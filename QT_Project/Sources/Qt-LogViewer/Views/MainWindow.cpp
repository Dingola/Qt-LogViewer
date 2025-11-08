#include "Qt-LogViewer/Views/MainWindow.h"

#include <QAction>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QIcon>
#include <QItemSelectionModel>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QSet>
#include <QStringList>
#include <QTimer>
#include <QUrl>

#include "Qt-LogViewer/Models/LogModel.h"
#include "Qt-LogViewer/Models/LogSortFilterProxyModel.h"
#include "Qt-LogViewer/Services/LogViewerSettings.h"
#include "Qt-LogViewer/Services/Translator.h"
#include "Qt-LogViewer/Views/App/Dialogs/SettingsDialog.h"
#include "Qt-LogViewer/Views/App/LogTableView.h"
#include "ui_MainWindow.h"

namespace
{
constexpr auto k_open_log_files_text = QT_TRANSLATE_NOOP("MainWindow", "Open Log Files");
constexpr auto k_open_log_file_text = QT_TRANSLATE_NOOP("MainWindow", "Open Log File...");
constexpr auto k_file_menu_text = QT_TRANSLATE_NOOP("MainWindow", "&File");
constexpr auto k_loaded_log_files_status = QT_TRANSLATE_NOOP("MainWindow", "Loaded %1 log file(s)");
constexpr auto k_quit_text = QT_TRANSLATE_NOOP("MainWindow", "&Quit");
constexpr auto k_views_menu_text = QT_TRANSLATE_NOOP("MainWindow", "&Views");
constexpr auto k_show_log_file_explorer_text =
    QT_TRANSLATE_NOOP("MainWindow", "Show Log File Explorer");
constexpr auto k_show_log_details_text = QT_TRANSLATE_NOOP("MainWindow", "Show Log Details");
}  // namespace

/**
 * @brief Constructs a MainWindow object.
 *
 * Initializes the main window, sets up the user interface, menu, status bar, and connects all
 * signals/slots.
 *
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

    m_controller->set_current_view(m_controller->load_log_files({}));
    connect(m_controller, &LogViewerController::current_view_id_changed, this,
            &MainWindow::handle_current_view_id_changed);
    connect(m_controller, &LogViewerController::view_removed, this,
            &MainWindow::handle_view_removed);

    setup_log_file_explorer();
    setup_log_level_pie_chart();
    setup_pagination_widget();
    setup_log_details_dock();
    setup_filter_bar();
    setup_tab_widget();

    initialize_menu();

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
    connect(m_log_file_explorer, &LogFileExplorer::remove_file_requested, m_controller,
            [this](const LogFileInfo& log_file_info) {
                m_controller->remove_log_file(log_file_info);
                update_pagination_widget();
            });
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
    connect(ui->logFilterBarWidget, &LogFilterBarWidget::search_requested, this,
            &MainWindow::handle_search_changed);
    connect(ui->logFilterBarWidget, &LogFilterBarWidget::search_text_changed, this,
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

    connect(ui->tabWidgetLog, &QTabWidget::currentChanged, this, [this](int index) {
        auto* log_table_view = qobject_cast<LogTableView*>(ui->tabWidgetLog->widget(index));
        if (log_table_view != nullptr)
        {
            QUuid view_id = log_table_view->property("view_id").value<QUuid>();
            m_controller->set_current_view(view_id);
            update_pagination_widget();
        }
    });
    connect(ui->tabWidgetLog, &QTabWidget::tabCloseRequested, this, [this](int index) {
        auto* log_table_view = qobject_cast<LogTableView*>(ui->tabWidgetLog->widget(index));
        if (log_table_view != nullptr)
        {
            QUuid view_id = log_table_view->property("view_id").value<QUuid>();
            m_controller->remove_view(view_id);
        }
        ui->tabWidgetLog->removeTab(index);
        delete log_table_view;

        // Update pagination when last tab is closed (not handled by handle_current_view_id_changed)
        if (ui->tabWidgetLog->count() == 0)
        {
            ui->logFilterBarWidget->set_app_names({});
            ui->logFilterBarWidget->set_log_levels({});
            m_log_level_pie_chart_widget->set_log_level_counts({});
            ui->logFilterBarWidget->set_log_level_counts({});
            update_pagination_widget();
        }
    });
}

/**
 * @brief Initializes the main menu bar and its actions.
 */
auto MainWindow::initialize_menu() -> void
{
    // File menu
    auto file_menu = new QMenu(tr(k_file_menu_text), this);

    // Action to open log files
    m_action_open_log_file = new QAction(tr(k_open_log_file_text), this);
    m_action_open_log_file->setShortcut(QKeySequence::Open);
    file_menu->addAction(m_action_open_log_file);
    ui->menubar->addMenu(file_menu);

    connect(m_action_open_log_file, &QAction::triggered, this,
            &MainWindow::handle_open_log_file_dialog_requested);

    // Separator before the quit action
    file_menu->addSeparator();
    // Action to quit the application
    m_action_quit = new QAction(tr(k_quit_text), this);
#ifdef Q_OS_WIN
    m_action_quit->setShortcut(QKeySequence(QStringLiteral("Ctrl+Q")));
#else
    m_action_quit->setShortcut(QKeySequence::Quit);
#endif
    file_menu->addAction(m_action_quit);
    ui->menubar->addMenu(file_menu);

    connect(m_action_quit, &QAction::triggered, this, &QWidget::close);

    // Views menu
    auto views_menu = new QMenu(tr(k_views_menu_text), this);

    m_action_show_log_file_explorer = new QAction(tr(k_show_log_file_explorer_text), this);
    m_action_show_log_file_explorer->setCheckable(true);
    m_action_show_log_file_explorer->setChecked(true);
    views_menu->addAction(m_action_show_log_file_explorer);

    m_action_show_log_details = new QAction(tr(k_show_log_details_text), this);
    m_action_show_log_details->setCheckable(true);
    m_action_show_log_details->setChecked(true);
    views_menu->addAction(m_action_show_log_details);

    m_action_show_log_level_pie_chart = new QAction(tr("Show Log Level Pie Chart"), this);
    m_action_show_log_level_pie_chart->setCheckable(true);
    m_action_show_log_level_pie_chart->setChecked(true);
    views_menu->addAction(m_action_show_log_level_pie_chart);

    ui->menubar->addMenu(views_menu);

    connect(m_action_show_log_file_explorer, &QAction::toggled, this,
            [this](bool checked) { m_log_file_explorer_dock_widget->setVisible(checked); });
    connect(m_log_file_explorer_dock_widget, &DockWidget::closed, this,
            [this]() { m_action_show_log_file_explorer->setChecked(false); });

    connect(m_action_show_log_details, &QAction::toggled, this,
            [this](bool checked) { m_log_details_dock_widget->setVisible(checked); });
    connect(m_log_details_dock_widget, &DockWidget::closed, this,
            [this]() { m_action_show_log_details->setChecked(false); });

    connect(m_action_show_log_level_pie_chart, &QAction::toggled, this,
            [this](bool checked) { m_log_level_pie_chart_dock_widget->setVisible(checked); });
    connect(m_log_level_pie_chart_dock_widget, &DockWidget::closed, this,
            [this]() { m_action_show_log_level_pie_chart->setChecked(false); });

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
    auto* proxy = m_controller->get_paging_proxy();
    int total_pages = 0;
    int current_page = 0;

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
    m_controller->add_log_files_to_tree(files);
}

/**
 * @brief Handles resize events to adjust the layout.
 * @param event The resize event.
 */
void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);

    if (ui != nullptr)
    {
        auto* log_table_view = qobject_cast<LogTableView*>(ui->tabWidgetLog->currentWidget());

        if (log_table_view != nullptr)
        {
            log_table_view->auto_resize_columns();
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
        m_log_file_explorer_dock_widget->setWindowTitle(tr("Log File Explorer"));
        m_log_details_dock_widget->setWindowTitle(tr("Log Details"));
        ui->logFilterBarWidget->set_app_names(m_controller->get_app_names());
        update_pagination_widget();
    }

    BaseMainWindow::changeEvent(event);
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
    m_controller->add_log_files_to_tree(files);
}

/**
 * @brief Opens the settings dialog for changing application settings.
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
 * @brief Handles open log file requests and creates a new tab with a LogTableView.
 * @param log_file_info The LogFileInfo to load and display.
 */
auto MainWindow::handle_log_file_open_requested(const LogFileInfo& log_file_info) -> void
{
    auto view_id = m_controller->load_log_file(log_file_info.get_file_path());
    m_controller->set_current_view(view_id);

    auto* log_table_view = new LogTableView(ui->tabWidgetLog);
    log_table_view->setProperty("view_id", QVariant::fromValue(view_id));
    auto* paging_proxy = m_controller->get_paging_proxy(view_id);
    log_table_view->setModel(paging_proxy);
    connect(log_table_view->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
            &MainWindow::update_log_details);

    QString tab_title = log_file_info.get_file_name();
    int tab_index = ui->tabWidgetLog->addTab(log_table_view, tab_title);
    ui->tabWidgetLog->setCurrentIndex(tab_index);
    log_table_view->auto_resize_columns();
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

    auto* log_table_view = qobject_cast<LogTableView*>(ui->tabWidgetLog->currentWidget());

    if (log_table_view != nullptr)
    {
        log_table_view->auto_resize_columns();
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
        auto* log_table_view = qobject_cast<LogTableView*>(ui->tabWidgetLog->widget(i));

        if (log_table_view != nullptr)
        {
            QUuid tab_view_id = log_table_view->property("view_id").value<QUuid>();

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
