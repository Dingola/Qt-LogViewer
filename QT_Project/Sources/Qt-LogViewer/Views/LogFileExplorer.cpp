#include "Qt-LogViewer/Views/LogFileExplorer.h"

#include <QTreeView>

#include "Qt-LogViewer/Models/LogFileInfo.h"
#include "Qt-LogViewer/Models/LogFileTreeItem.h"
#include "Qt-LogViewer/Models/LogFileTreeModel.h"
#include "ui_LogFileExplorer.h"

/**
 * @brief Constructs a LogFileExplorer widget.
 * @param parent The parent widget, or nullptr.
 */
LogFileExplorer::LogFileExplorer(LogFileTreeModel* model, QWidget* parent)
    : QWidget(parent),
      m_model(model != nullptr ? model : new LogFileTreeModel(this)),
      ui(new Ui::LogFileExplorer)
{
    ui->setupUi(this);
    ui->treeView->setModel(m_model);
    ui->treeView->setHeaderHidden(false);
    ui->treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->treeView->setExpandsOnDoubleClick(true);
    ui->treeView->setRootIsDecorated(true);
    setup_connections();
}

/**
 * @brief Destroys the LogFileExplorer widget.
 */
LogFileExplorer::~LogFileExplorer()
{
    delete ui;
}

/**
 * @brief Sets the model for the tree view.
 * @param model Pointer to the LogFileTreeModel to set.
 */
auto LogFileExplorer::set_model(LogFileTreeModel* model) -> void
{
    m_model = model;
    ui->treeView->setModel(m_model);
}

/**
 * @brief Returns the underlying tree view.
 * @return Pointer to the QTreeView.
 */
auto LogFileExplorer::tree_view() const -> QTreeView*
{
    return ui->treeView;
}

/**
 * @brief Returns the underlying model.
 * @return Pointer to the LogFileTreeModel.
 */
auto LogFileExplorer::model() const -> LogFileTreeModel*
{
    return m_model;
}

/**
 * @brief Sets the log files to display in the tree view.
 * @param files The list of LogFileInfo objects.
 */
auto LogFileExplorer::set_log_files(const QList<LogFileInfo>& files) -> void
{
    m_model->set_log_files(files);
}

/**
 * @brief Sets up connections for selection changes.
 */
auto LogFileExplorer::setup_connections() -> void
{
    connect(ui->treeView->selectionModel(), &QItemSelectionModel::currentChanged, this,
            [this](const QModelIndex& current, const QModelIndex&) {
                if (!current.isValid()) return;
                auto* item = static_cast<LogFileTreeItem*>(current.internalPointer());
                if (item && item->parent_item())
                {
                    emit logFileSelected(LogFileInfo(item->data(0).toString(),
                                                     item->parent_item()->data(0).toString()));
                }
            });
}

/**
 * @brief Adds a single log file to the tree view.
 * @param file The LogFileInfo to add.
 */
auto LogFileExplorer::add_log_file(const LogFileInfo& file) -> void
{
    m_model->add_log_file(file);
}

/**
 * @brief Removes a single log file from the tree view.
 * @param file The LogFileInfo to remove.
 */
auto LogFileExplorer::remove_log_file(const LogFileInfo& file) -> void
{
    m_model->remove_log_file(file);
}
