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

#include "Qt-LogViewer/Models/LogFilterProxyModel.h"
#include "Qt-LogViewer/Models/LogModel.h"
#include "Qt-LogViewer/Services/LogViewerSettings.h"
#include "Qt-LogViewer/Services/Translator.h"
#include "Qt-LogViewer/Views/LogTableView.h"
#include "Qt-LogViewer/Views/SettingsDialog.h"
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

    // Initialize the user interface
    ui->setupUi(this);
    setWindowIcon(QIcon(":/Resources/Icons/App/AppIcon.svg"));

    // Set up the log file explorer
    m_log_file_explorer = new LogFileExplorer(m_controller->get_log_file_tree_model(), this);
    m_log_file_explorer_dock_widget = new DockWidget(tr("Log File Explorer"), this);
    m_log_file_explorer_dock_widget->setTitleBarWidget(DockWidget::create_dock_title_bar(
        m_log_file_explorer_dock_widget, "logFileExplorerTitleBar"));
    m_log_file_explorer_dock_widget->setObjectName("logFileExplorerDockWidget");
    m_log_file_explorer_dock_widget->setWidget(m_log_file_explorer);
    addDockWidget(Qt::LeftDockWidgetArea, m_log_file_explorer_dock_widget);
    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
    connect(m_log_file_explorer, &LogFileExplorer::remove_requested, m_controller,
            [this](const LogFileInfo& log_file_info) {
                m_controller->remove_log_file(log_file_info);
                update_pagination_widget();
            });
    connect(m_log_file_explorer, &LogFileExplorer::file_selected, this,
            &MainWindow::update_log_level_pie_chart);

    // Set up the log level pie chart
    m_log_level_pie_chart_dock_widget = new DockWidget(tr("Log Level Pie Chart"), this);
    m_log_level_pie_chart_dock_widget->setTitleBarWidget(DockWidget::create_dock_title_bar(
        m_log_level_pie_chart_dock_widget, "logLevelPieChartTitleBar"));
    m_log_level_pie_chart_dock_widget->setObjectName("logLevelPieChartDockWidget");
    m_log_level_pie_chart_dock_widget->setWidget(m_log_level_pie_chart_widget);
    addDockWidget(Qt::LeftDockWidgetArea, m_log_level_pie_chart_dock_widget);
    setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
    setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);

    // Set up the model/view for the log table
    auto log_filter_proxy_model = m_controller->get_proxy_model();
    ui->tableViewLog->setModel(log_filter_proxy_model);

    // Set up pagination
    ui->paginationWidget->set_max_page_buttons(7);
    int items_per_page = 25;
    log_filter_proxy_model->set_page_size(items_per_page);
    log_filter_proxy_model->set_current_page(1);

    connect(ui->paginationWidget, &PaginationWidget::page_changed, this,
            [this](int page) { m_controller->get_proxy_model()->set_current_page(page); });
    connect(ui->paginationWidget, &PaginationWidget::items_per_page_changed, this,
            [this](int items_per_page) {
                auto* proxy = m_controller->get_proxy_model();
                proxy->set_page_size(items_per_page);
                proxy->set_current_page(1);
                update_pagination_widget();
            });

    // Create and set up the dock widget for log details
    m_log_details_dock_widget = new DockWidget(tr("Log Details"), this);
    m_log_details_dock_widget->setObjectName("logDetailsDockWidget");
    m_log_details_dock_widget->setTitleBarWidget(
        DockWidget::create_dock_title_bar(m_log_details_dock_widget, "logDetailsTitleBar"));
    m_log_details_text_edit = new QPlainTextEdit(m_log_details_dock_widget);
    m_log_details_text_edit->setObjectName("logDetailsTextEdit");
    m_log_details_text_edit->setReadOnly(true);
    auto* log_details_container = new QWidget(m_log_details_dock_widget);
    auto* log_details_layout = new QVBoxLayout(log_details_container);
    log_details_layout->setContentsMargins(8, 0, 8, 8);
    log_details_layout->addWidget(m_log_details_text_edit);
    log_details_container->setLayout(log_details_layout);
    m_log_details_dock_widget->setWidget(log_details_container);
    addDockWidget(Qt::BottomDockWidgetArea, m_log_details_dock_widget);

    initialize_menu();

    // Connect filter controls to controller
    connect(ui->filterBarWidget, &FilterBarWidget::app_filter_changed, this,
            [this](const QString& app_name) {
                m_controller->set_app_name_filter(app_name);
                update_pagination_widget();
            });
    connect(ui->filterBarWidget, &FilterBarWidget::log_level_filter_changed, this,
            [this](const QSet<QString>& levels) {
                qDebug() << "Level filter set to:" << levels;
                m_controller->set_level_filter(levels);
                update_pagination_widget();
            });
    connect(ui->filterBarWidget, &FilterBarWidget::search_requested, this,
            &MainWindow::on_search_changed);
    connect(ui->filterBarWidget, &FilterBarWidget::search_text_changed, this,
            &MainWindow::on_search_changed);
    connect(ui->filterBarWidget, &FilterBarWidget::search_field_changed, this,
            &MainWindow::on_search_changed);
    connect(ui->filterBarWidget, &FilterBarWidget::regex_toggled, this,
            &MainWindow::on_search_changed);

    // Connect table selection to log details view
    connect(ui->tableViewLog->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
            &MainWindow::update_log_details);

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

    connect(m_action_open_log_file, &QAction::triggered, this, &MainWindow::open_log_files);

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

    connect(m_action_settings, &QAction::triggered, this, &MainWindow::show_settings_dialog);

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
        auto* proxy = m_controller->get_proxy_model();
        auto* model = m_controller->get_log_model();
        QModelIndex source_index = proxy->mapToSource(current);
        LogEntry entry = model->get_entry(source_index.row());
        details = QString("Timestamp: %1\nLevel: %2\nApp: %3\nMessage: %4")
                      .arg(entry.get_timestamp().toString("yyyy-MM-dd HH:mm:ss"))
                      .arg(entry.get_level())
                      .arg(entry.get_app_name())
                      .arg(entry.get_message());
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
    auto* proxy = m_controller->get_proxy_model();
    int total_pages = proxy->get_total_pages();
    int current_page = proxy->get_current_page();

    if (total_pages < 1)
    {
        total_pages = 1;
    }
    if (current_page < 1)
    {
        current_page = 1;
    }
    if (current_page > total_pages)
    {
        current_page = total_pages;
    }

    ui->paginationWidget->set_pagination(current_page, total_pages);
}

/**
 * @brief Loads log files and updates the UI.
 * @param files The list of log file paths to load.
 *
 * This method loads the specified log files into the controller, extracts unique application names
 * from the loaded logs, updates the application combo box, and shows a status message indicating
 * how many files were loaded.
 */
auto MainWindow::load_files_and_update_ui(const QStringList& files) -> void
{
    qDebug() << "Requested to load files:" << files;

    if (!files.isEmpty())
    {
        QVector<QString> file_paths = files.toVector();
        m_controller->load_logs(file_paths);
        QSet<QString> app_names = m_controller->get_app_names();
        ui->filterBarWidget->set_app_names(app_names);
        qDebug() << "Loaded" << app_names.size() << "apps from" << files.size() << "files.";

        update_pagination_widget();
        statusBar()->showMessage(tr(k_loaded_log_files_status).arg(files.size()), 3000);
    }
    else
    {
        qWarning() << "No files provided to load_files_and_update_ui.";
    }
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
    load_files_and_update_ui(files);
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
        ui->tableViewLog->auto_resize_columns();
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
        ui->filterBarWidget->set_app_names(m_controller->get_app_names());
        update_pagination_widget();
    }

    BaseMainWindow::changeEvent(event);
}

/**
 * @brief Opens log files using a file dialog and loads them into the controller.
 */
void MainWindow::open_log_files()
{
    qDebug() << "Opening log file dialog";
    QStringList files = QFileDialog::getOpenFileNames(this, tr(k_open_log_files_text), QString(),
                                                      tr("Log Files (*.log *.txt);;All Files (*)"));
    qDebug() << "Files selected:" << files;
    load_files_and_update_ui(files);
}

/**
 * @brief Opens the settings dialog for changing application settings.
 */
void MainWindow::show_settings_dialog()
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
auto MainWindow::on_search_changed() -> void
{
    QString search_text = ui->filterBarWidget->get_search_text();
    QString field = ui->filterBarWidget->get_search_field();
    bool use_regex = ui->filterBarWidget->get_use_regex();
    qDebug() << "Search filter:" << search_text << "Field:" << field << "Regex:" << use_regex;
    m_controller->set_search_filter(search_text, field, use_regex);
    update_pagination_widget();
}

/**
 * @brief Updates the log level pie chart for the selected file.
 * @param log_file_info The selected LogFileInfo.
 */
auto MainWindow::update_log_level_pie_chart(const LogFileInfo& log_file_info) -> void
{
    QVector<LogEntry> entries = m_controller->get_entries_for_file(log_file_info);

    QMap<QString, int> level_counts;

    for (const auto& entry: entries)
    {
        level_counts[entry.get_level()]++;
    }

    m_log_level_pie_chart_widget->set_log_level_counts(level_counts);
}
