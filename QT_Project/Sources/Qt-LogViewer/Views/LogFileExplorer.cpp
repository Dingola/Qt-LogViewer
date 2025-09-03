#include "Qt-LogViewer/Views/LogFileExplorer.h"

#include <QCursor>
#include <QModelIndex>
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
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    setup_connections();
    init_context_menu();
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
 * @param log_file_infos The list of LogFileInfo objects.
 */
auto LogFileExplorer::set_log_files(const QList<LogFileInfo>& log_file_infos) -> void
{
    m_model->set_log_files(log_file_infos);
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
                    emit file_selected(LogFileInfo(item->data(0).toString(),
                                                   item->parent_item()->data(0).toString()));
                }
            });
    connect(ui->treeView, &QTreeView::customContextMenuRequested, this,
            &LogFileExplorer::show_context_menu);
}

/**
 * @brief Initializes the context menu for the tree view.
 */
auto LogFileExplorer::init_context_menu() -> void
{
    m_context_menu = new QMenu(this);
    QAction* remove_action = new QAction(tr("Remove"), m_context_menu);
    m_context_menu->addAction(remove_action);
    connect(remove_action, &QAction::triggered, this, [this]() {
        QModelIndex index = ui->treeView->currentIndex();
        if (index.isValid())
        {
            auto* item = static_cast<LogFileTreeItem*>(index.internalPointer());

            if (item != nullptr)
            {
                emit remove_requested(item->data(1).value<LogFileInfo>());
            }
        }
    });
}

/**
 * @brief Handles context menu requests on the tree view.
 * @param pos The position where the menu should appear.
 */
auto LogFileExplorer::show_context_menu(const QPoint& pos) -> void
{
    QModelIndex index = ui->treeView->indexAt(pos);

    if (index.isValid())
    {
        auto* item = static_cast<LogFileTreeItem*>(index.internalPointer());

        if (item != nullptr &&
            item->data(0).value<LogFileTreeItem::Type>() == LogFileTreeItem::Type::File)
        {
            m_context_menu->exec(ui->treeView->viewport()->mapToGlobal(pos));
        }
    }
}

/**
 * @brief Handles change events to update the UI.
 * @param event The change event.
 */
auto LogFileExplorer::changeEvent(QEvent* event) -> void
{
    if (event != nullptr && event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        init_context_menu();
    }

    QWidget::changeEvent(event);
}
