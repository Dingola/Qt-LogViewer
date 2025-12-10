#include "Qt-LogViewer/Views/App/LogFileExplorer.h"

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
    ui->treeView->setHeaderHidden(true);
    ui->treeView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->treeView->setExpandsOnDoubleClick(true);
    ui->treeView->setRootIsDecorated(true);
    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->treeView->setTextElideMode(Qt::ElideMiddle);
    setAttribute(Qt::WA_StyledBackground, true);
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
                if (current.isValid())
                {
                    auto* item = static_cast<LogFileTreeItem*>(current.internalPointer());
                    if (item != nullptr &&
                        item->data(0).value<LogFileTreeItem::Type>() == LogFileTreeItem::Type::File)
                    {
                        emit file_selected(item->data(1).value<LogFileInfo>());
                    }
                }
            });
    connect(ui->treeView, &QTreeView::doubleClicked, this, [this](const QModelIndex& index) {
        if (index.isValid())
        {
            auto* item = static_cast<LogFileTreeItem*>(index.internalPointer());
            if (item != nullptr &&
                item->data(0).value<LogFileTreeItem::Type>() == LogFileTreeItem::Type::File)
            {
                emit open_file_requested(item->data(1).value<LogFileInfo>());
            }
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

    auto open_action = new QAction(tr("Open in New View"), m_context_menu);
    m_context_menu->addAction(open_action);
    connect(open_action, &QAction::triggered, this, [this]() {
        auto dispatched = dispatch_current_if_type(
            LogFileTreeItem::Type::File,
            [this](const LogFileInfo& info) { emit open_file_requested(info); });
        Q_UNUSED(dispatched);
    });

    auto add_to_current_view_action = new QAction(tr("Add to Current View"), m_context_menu);
    m_context_menu->addAction(add_to_current_view_action);
    connect(add_to_current_view_action, &QAction::triggered, this, [this]() {
        auto dispatched = dispatch_current_if_type(
            LogFileTreeItem::Type::File,
            [this](const LogFileInfo& info) { emit add_to_current_view_requested(info); });
        Q_UNUSED(dispatched);
    });

    auto remove_action = new QAction(tr("Remove"), m_context_menu);
    m_context_menu->addAction(remove_action);
    connect(remove_action, &QAction::triggered, this, [this]() {
        auto dispatched = dispatch_current_if_type(
            LogFileTreeItem::Type::File,
            [this](const LogFileInfo& info) { emit remove_file_requested(info); });
        Q_UNUSED(dispatched);
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
 * @brief Tries to extract a LogFileInfo for the given index if the item type matches.
 * @param index The model index to inspect.
 * @param expected_type The expected LogFileTreeItem::Type (e.g., File, Group).
 * @param out_info Output parameter that receives the LogFileInfo on success.
 * @return True if the index is valid, the item type matches, and out_info was set; false otherwise.
 */
[[nodiscard]] auto LogFileExplorer::try_get_info_if_type(const QModelIndex& index,
                                                         LogFileTreeItem::Type expected_type,
                                                         LogFileInfo& out_info) const -> bool
{
    bool success = false;

    if (index.isValid())
    {
        auto* item = static_cast<LogFileTreeItem*>(index.internalPointer());

        if (item != nullptr)
        {
            const auto type_variant = item->data(0);
            if (type_variant.canConvert<LogFileTreeItem::Type>())
            {
                const auto item_type = type_variant.value<LogFileTreeItem::Type>();

                if (item_type == expected_type)
                {
                    out_info = item->data(1).value<LogFileInfo>();
                    success = true;
                }
            }
        }
    }

    return success;
}

/**
 * @brief Dispatches a callable for the current index if the item type matches.
 * @param expected_type The expected LogFileTreeItem::Type.
 * @param fn The callable to invoke with LogFileInfo when type matches.
 * @return True if dispatched; false otherwise.
 */
[[nodiscard]] auto LogFileExplorer::dispatch_current_if_type(
    LogFileTreeItem::Type expected_type,
    const std::function<void(const LogFileInfo&)>& fn) const -> bool
{
    bool dispatched = false;

    const QModelIndex index = ui->treeView->currentIndex();
    LogFileInfo info;

    if (try_get_info_if_type(index, expected_type, info))
    {
        fn(info);
        dispatched = true;
    }

    return dispatched;
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
