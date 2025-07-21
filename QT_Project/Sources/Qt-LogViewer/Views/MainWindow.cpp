#include "Qt-LogViewer/Views/MainWindow.h"

#include <QAction>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QFileDialog>
#include <QItemSelectionModel>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QSet>
#include <QStringList>
#include <QTimer>
#include <QUrl>

#include "Qt-LogViewer/Views/HoverRowDelegate.h"
#include "Qt-LogViewer/Views/TableView.h"
#include "ui_MainWindow.h"

namespace
{
constexpr auto k_show_all_apps_text = QT_TRANSLATE_NOOP("MainWindow", "Show All Apps");
constexpr auto k_show_all_apps_tooltip =
    QT_TRANSLATE_NOOP("MainWindow", "Show logs from all applications");
constexpr auto k_open_log_files_text = QT_TRANSLATE_NOOP("MainWindow", "Open Log Files");
constexpr auto k_open_log_file_text = QT_TRANSLATE_NOOP("MainWindow", "Open Log File...");
constexpr auto k_file_menu_text = QT_TRANSLATE_NOOP("MainWindow", "&File");
constexpr auto k_loaded_log_files_status = QT_TRANSLATE_NOOP("MainWindow", "Loaded %1 log file(s)");
constexpr auto k_search_placeholder_text = QT_TRANSLATE_NOOP("MainWindow", "Enter search text...");
}  // namespace

/**
 * @brief Constructs a MainWindow object.
 *
 * Initializes the main window, sets up the user interface, menu, status bar, and connects all
 * signals/slots.
 *
 * @param parent The parent widget, or nullptr if this is a top-level window.
 */
MainWindow::MainWindow(QWidget* parent): QMainWindow(parent), ui(new Ui::MainWindow)
{
    qDebug() << "MainWindow constructor started";

    // Initialize the user interface
    ui->setupUi(this);

    // Load and apply application stylesheet from Qt resource
    QFile style_file(":/Resources/style.qss");
    if (style_file.open(QFile::ReadOnly | QFile::Text))
    {
        QString style_sheet = QString::fromUtf8(style_file.readAll());
        qApp->setStyleSheet(style_sheet);
        qDebug() << "Loaded stylesheet from resource";
    }
    else
    {
        qWarning() << "Failed to load stylesheet from resource:" << style_file.errorString();
    }

    // Init combo box for app names
    update_app_combo_box({});

    initialize_menu();

    // Initialize controller with a default log format string
    m_controller = new LogViewerController("{timestamp} {level} {message} {app_name}", this);

    ui->lineEditSearch->setPlaceholderText(tr(k_search_placeholder_text));

    // Set up the model/view for the log table
    auto log_filter_proxy_model = m_controller->get_proxy_model();
    ui->tableViewLog->setModel(log_filter_proxy_model);
    ui->tableViewLog->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableViewLog->setSelectionMode(QAbstractItemView::SingleSelection);
    auto* header = ui->tableViewLog->horizontalHeader();
    int column_count = ui->tableViewLog->model()->columnCount();
    for (int i = 0; i < column_count; ++i)
    {
        if (i == (column_count - 1))
        {
            header->setSectionResizeMode(i, QHeaderView::Stretch);
        }
        else
        {
            header->setSectionResizeMode(i, QHeaderView::Interactive);
        }
    }

    auto hover_delegate = new HoverRowDelegate(ui->tableViewLog);
    ui->tableViewLog->setItemDelegate(hover_delegate);
    ui->tableViewLog->setMouseTracking(true);
    connect(ui->tableViewLog, &TableView::hover_index_changed, hover_delegate,
            [hover_delegate](const QModelIndex& index) {
                hover_delegate->set_hovered_row(index.isValid() ? index.row() : -1);
                if (auto* view = qobject_cast<QAbstractItemView*>(hover_delegate->parent()))
                {
                    view->viewport()->update();
                }
            });

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

    // Populate search area combo box
    ui->comboBoxSearchArea->addItems(QStringList()
                                     << "All Fields" << "Message" << "Level" << "AppName");

    ui->comboBoxApp->setFixedWidth(140);

    // Create and set up the dock widget for log details
    m_log_details_dock_widget = new QDockWidget(tr("Log Details"), this);
    m_log_details_text_edit = new QPlainTextEdit(m_log_details_dock_widget);
    m_log_details_text_edit->setReadOnly(true);
    m_log_details_dock_widget->setWidget(m_log_details_text_edit);
    addDockWidget(Qt::BottomDockWidgetArea, m_log_details_dock_widget);

    // Connect filter controls to controller
    connect(ui->comboBoxApp, &QComboBox::currentTextChanged, this, [this](const QString& app_name) {
        m_controller->set_app_name_filter(
            (app_name == tr(k_show_all_apps_text) ? QString() : app_name));
        update_pagination_widget();
    });

    connect(ui->checkBoxTrace, &QCheckBox::toggled, this, &MainWindow::update_level_filter);
    connect(ui->checkBoxDebug, &QCheckBox::toggled, this, &MainWindow::update_level_filter);
    connect(ui->checkBoxInfo, &QCheckBox::toggled, this, &MainWindow::update_level_filter);
    connect(ui->checkBoxWarning, &QCheckBox::toggled, this, &MainWindow::update_level_filter);
    connect(ui->checkBoxError, &QCheckBox::toggled, this, &MainWindow::update_level_filter);
    connect(ui->checkBoxFatal, &QCheckBox::toggled, this, &MainWindow::update_level_filter);

    connect(ui->lineEditSearch, &QLineEdit::textChanged,
            [=] { style()->polish(ui->lineEditSearch); });
    connect(ui->lineEditSearch, &QLineEdit::textChanged, this, &MainWindow::update_search_filter);
    connect(ui->comboBoxSearchArea, &QComboBox::currentTextChanged, this,
            &MainWindow::update_search_filter);
    connect(ui->checkBoxRegEx, &QCheckBox::toggled, this, &MainWindow::update_search_filter);

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
    delete m_controller;
    delete ui;
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
 * @brief Initializes the main menu bar and its actions.
 */
auto MainWindow::initialize_menu() -> void
{
    // File menu
    auto file_menu = new QMenu(tr(k_file_menu_text), this);
    m_action_open_log_file = new QAction(tr(k_open_log_file_text), this);
    file_menu->addAction(m_action_open_log_file);
    ui->menubar->addMenu(file_menu);

    connect(m_action_open_log_file, &QAction::triggered, this, &MainWindow::open_log_files);

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
 * @brief Updates the log level filter in the controller.
 */
auto MainWindow::update_level_filter() -> void
{
    QSet<QString> levels;

    if (ui->checkBoxTrace->isChecked())
    {
        levels.insert("TRACE");
    }
    if (ui->checkBoxDebug->isChecked())
    {
        levels.insert("DEBUG");
    }
    if (ui->checkBoxInfo->isChecked())
    {
        levels.insert("INFO");
    }
    if (ui->checkBoxWarning->isChecked())
    {
        levels.insert("WARNING");
    }
    if (ui->checkBoxError->isChecked())
    {
        levels.insert("ERROR");
    }
    if (ui->checkBoxFatal->isChecked())
    {
        levels.insert("FATAL");
    }

    qDebug() << "Level filter set to:" << levels;
    m_controller->set_level_filter(levels);
    update_pagination_widget();
}

/**
 * @brief Updates the search filter in the controller.
 */
auto MainWindow::update_search_filter() -> void
{
    QString search_text = ui->lineEditSearch->text();
    QString field = ui->comboBoxSearchArea->currentText();
    bool use_regex = ui->checkBoxRegEx->isChecked();
    qDebug() << "Search filter:" << search_text << "Field:" << field << "Regex:" << use_regex;
    m_controller->set_search_filter(search_text, field, use_regex);
    update_pagination_widget();
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
 * @brief Updates the application combo box with the given application names.
 *
 * This method clears the combo box, adds a default entry and populates it
 * with the provided application names. It also disables the combo box if no application
 * names are provided.
 *
 * @param app_names A set of application names to populate the combo box.
 */
auto MainWindow::update_app_combo_box(const QSet<QString>& app_names) -> void
{
    ui->comboBoxApp->blockSignals(true);
    ui->comboBoxApp->clear();
    ui->comboBoxApp->addItem(tr(k_show_all_apps_text));
    ui->comboBoxApp->setItemData(0, tr(k_show_all_apps_tooltip), Qt::ToolTipRole);

    for (const QString& app: app_names)
    {
        if (!app.isEmpty())
        {
            ui->comboBoxApp->addItem(app);
        }
    }

    ui->comboBoxApp->setCurrentIndex(0);
    ui->comboBoxApp->setEnabled(!app_names.isEmpty());
    ui->comboBoxApp->blockSignals(false);
    qDebug() << "App combo box updated with" << app_names.size() << "apps.";
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
        update_app_combo_box(app_names);

        qDebug() << "Loaded" << app_names.size() << "apps from" << files.size() << "files.";
        update_app_combo_box(app_names);
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
 * @brief Resizes the table columns based on the window size.
 *
 * This method adjusts the column widths of the log table view when the window is resized.
 * It calculates the widths based on a percentage of the total width of the viewport.
 *
 * @param event The resize event containing the new size of the window.
 */
void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    bool valid_model = ui->tableViewLog->model() && ui->tableViewLog->model()->columnCount() >= 4;

    if (valid_model)
    {
        int total_width = ui->tableViewLog->viewport()->width();
        int col_0_width = static_cast<int>(total_width * 0.15);
        int col_1_width = static_cast<int>(total_width * 0.10);
        int col_2_width = static_cast<int>(total_width * 0.50);
        int col_3_width = static_cast<int>(total_width * 0.15);

        ui->tableViewLog->setColumnWidth(LogModel::Timestamp, col_0_width);
        ui->tableViewLog->setColumnWidth(LogModel::Level, col_1_width);
        ui->tableViewLog->setColumnWidth(LogModel::Message, col_2_width);
        ui->tableViewLog->setColumnWidth(LogModel::AppName, col_3_width);
#ifdef QT_DEBUG_VERBOSE
        qDebug() << "Resizing columns to widths:" << col_0_width << col_1_width << col_2_width
                 << col_3_width;
#endif
    }
    else
    {
        qWarning() << "Table model invalid or has too few columns!";
    }
}
