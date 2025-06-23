#include "Qt-LogViewer/Views/MainWindow.h"

#include <QDir>
#include <QItemSelectionModel>
#include <QSet>
#include <QStringList>

#include "ui_MainWindow.h"

/**
 * @brief Constructs a MainWindow object.
 *
 * Initializes the main window and sets up the user interface.
 *
 * @param parent The parent widget, or nullptr if this is a top-level window.
 */
MainWindow::MainWindow(QWidget* parent): QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Initialize controller with a default log format string
    m_controller = new LogViewerController("{timestamp} {level} {message} {app_name}", this);

    // Set up the model/view
    ui->tableViewLog->setModel(m_controller->get_proxy_model());
    ui->tableViewLog->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableViewLog->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableViewLog->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Populate search area combo box
    ui->comboBoxSearchArea->addItems(QStringList() << "Message" << "Level" << "AppName");

    // Connect filter controls
    connect(ui->comboBoxApp, &QComboBox::currentTextChanged, m_controller,
            &LogViewerController::set_app_name_filter);

    connect(ui->checkBoxTrace, &QCheckBox::toggled, this, &MainWindow::update_level_filter);
    connect(ui->checkBoxDebug, &QCheckBox::toggled, this, &MainWindow::update_level_filter);
    connect(ui->checkBoxInfo, &QCheckBox::toggled, this, &MainWindow::update_level_filter);
    connect(ui->checkBoxWarning, &QCheckBox::toggled, this, &MainWindow::update_level_filter);
    connect(ui->checkBoxError, &QCheckBox::toggled, this, &MainWindow::update_level_filter);
    connect(ui->checkBoxFatal, &QCheckBox::toggled, this, &MainWindow::update_level_filter);

    connect(ui->lineEditSearch, &QLineEdit::textChanged, this, &MainWindow::update_level_filter);
    connect(ui->comboBoxSearchArea, &QComboBox::currentTextChanged, this,
            &MainWindow::update_level_filter);
    connect(ui->checkBoxRegEx, &QCheckBox::toggled, this, &MainWindow::update_level_filter);

    // Connect table selection to log details view
    connect(ui->tableViewLog->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
            &MainWindow::update_log_details);

    /*
    QString log_dir = R"(C:\Users\Adrian\Desktop\Logs)";
    QDir dir(log_dir);
    QStringList log_files = dir.entryList(QStringList() << "*.log" << "*.txt", QDir::Files);
    QVector<QString> file_paths;
    for (const QString& file: log_files)
    {
        file_paths.append(dir.absoluteFilePath(file));
    }
    if (!file_paths.isEmpty())
    {
        m_controller->load_logs(file_paths);
    }
    */
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

    ui->plainTextEditLogDetails->setPlainText(details);
}
