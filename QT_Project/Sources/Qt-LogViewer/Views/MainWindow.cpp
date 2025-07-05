#include "Qt-LogViewer/Views/MainWindow.h"

#include <QAction>
#include <QFileDialog>
#include <QItemSelectionModel>
#include <QMenu>
#include <QSet>
#include <QStringList>

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
    // Initialize the user interface
    ui->setupUi(this);

    // Load and apply application stylesheet from Qt resource
    QFile style_file(":/Resources/style.qss");
    if (style_file.open(QFile::ReadOnly | QFile::Text))
    {
        QString style_sheet = QString::fromUtf8(style_file.readAll());
        qApp->setStyleSheet(style_sheet);
    }

    // Init combo box for app names
    update_app_combo_box({});

    // Create File menu and Open Log File action
    auto file_menu = new QMenu(tr(k_file_menu_text), this);
    m_action_open_log_file = new QAction(tr(k_open_log_file_text), this);
    file_menu->addAction(m_action_open_log_file);
    ui->menubar->addMenu(file_menu);

    // Connect menu action to file open slot
    connect(m_action_open_log_file, &QAction::triggered, this, &MainWindow::open_log_files);

    // Initialize controller with a default log format string
    m_controller = new LogViewerController("{timestamp} {level} {message} {app_name}", this);

    // Set up the model/view for the log table
    ui->tableViewLog->setModel(m_controller->get_proxy_model());
    ui->tableViewLog->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableViewLog->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableViewLog->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Populate search area combo box
    ui->comboBoxSearchArea->addItems(QStringList() << "Message" << "Level" << "AppName");

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
    });

    connect(ui->checkBoxTrace, &QCheckBox::toggled, this, &MainWindow::update_level_filter);
    connect(ui->checkBoxDebug, &QCheckBox::toggled, this, &MainWindow::update_level_filter);
    connect(ui->checkBoxInfo, &QCheckBox::toggled, this, &MainWindow::update_level_filter);
    connect(ui->checkBoxWarning, &QCheckBox::toggled, this, &MainWindow::update_level_filter);
    connect(ui->checkBoxError, &QCheckBox::toggled, this, &MainWindow::update_level_filter);
    connect(ui->checkBoxFatal, &QCheckBox::toggled, this, &MainWindow::update_level_filter);

    connect(ui->lineEditSearch, &QLineEdit::textChanged, this, &MainWindow::update_search_filter);
    connect(ui->comboBoxSearchArea, &QComboBox::currentTextChanged, this,
            &MainWindow::update_search_filter);
    connect(ui->checkBoxRegEx, &QCheckBox::toggled, this, &MainWindow::update_search_filter);

    // Connect table selection to log details view
    connect(ui->tableViewLog->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
            &MainWindow::update_log_details);
}

/**
 * @brief Destroys the MainWindow object.
 *
 * Cleans up any resources used by the main window.
 */
MainWindow::~MainWindow()
{
    delete m_controller;
    delete ui;
}

/**
 * @brief Opens log files using a file dialog and loads them into the controller.
 */
void MainWindow::open_log_files()
{
    QStringList files = QFileDialog::getOpenFileNames(this, tr(k_open_log_files_text), QString(),
                                                      tr("Log Files (*.log *.txt);;All Files (*)"));

    if (!files.isEmpty())
    {
        QVector<QString> file_paths = files.toVector();
        QSet<QString> app_names;
        m_controller->load_logs(file_paths);

        for (const LogEntry& entry: m_controller->get_log_model()->get_entries())
        {
            app_names.insert(entry.get_app_name());
        }

        update_app_combo_box(app_names);
        statusBar()->showMessage(tr(k_loaded_log_files_status).arg(files.size()), 3000);
    }
    else
    {
        update_app_combo_box({});
    }
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

    m_controller->set_level_filter(levels);
}

/**
 * @brief Updates the search filter in the controller.
 */
auto MainWindow::update_search_filter() -> void
{
    QString search_text = ui->lineEditSearch->text();
    QString field = ui->comboBoxSearchArea->currentText();
    bool use_regex = ui->checkBoxRegEx->isChecked();
    m_controller->set_search_filter(search_text, field, use_regex);
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
}
