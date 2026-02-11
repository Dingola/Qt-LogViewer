#include "Qt-LogViewer/Views/App/LogFileExplorer.h"

#include <QCursor>
#include <QInputDialog>
#include <QMessageBox>
#include <QModelIndex>
#include <QTreeView>

#include "Qt-LogViewer/Models/LogFileInfo.h"
#include "Qt-LogViewer/Models/LogFileTreeItem.h"
#include "Qt-LogViewer/Models/LogFileTreeModel.h"
#include "ui_LogFileExplorer.h"

/**
 * @brief Constructs a LogFileExplorer widget.
 * @param model The LogFileTreeModel to use, or nullptr to create a new one.
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
 * @brief Expands the tree to show a specific session.
 * @param session_id The session identifier to expand.
 */
auto LogFileExplorer::expand_session(const QString& session_id) -> void
{
    const QModelIndex session_index = m_model->get_session_index(session_id);
    if (session_index.isValid())
    {
        ui->treeView->expand(session_index);
    }
}

/**
 * @brief Sets up connections for signals and slots.
 */
auto LogFileExplorer::setup_connections() -> void
{
    connect(ui->treeView->selectionModel(), &QItemSelectionModel::currentChanged, this,
            [this](const QModelIndex& current, const QModelIndex&) {
                if (current.isValid())
                {
                    const auto type = get_item_type(current);

                    if (type == LogFileTreeItem::Type::File)
                    {
                        auto* item = static_cast<LogFileTreeItem*>(current.internalPointer());
                        emit file_selected(item->data(1).value<LogFileInfo>());
                    }
                    else if (type == LogFileTreeItem::Type::Session)
                    {
                        const QString session_id =
                            current.data(LogFileTreeModel::SessionIdRole).toString();
                        emit session_selected(session_id);
                    }
                }
            });

    connect(ui->treeView, &QTreeView::doubleClicked, this, [this](const QModelIndex& index) {
        if (index.isValid())
        {
            const auto type = get_item_type(index);

            if (type == LogFileTreeItem::Type::File)
            {
                auto* item = static_cast<LogFileTreeItem*>(index.internalPointer());
                emit open_file_requested(item->data(1).value<LogFileInfo>());
            }
            // Note: Session double-click triggers edit mode via Qt::ItemIsEditable flag
        }
    });

    connect(ui->treeView, &QTreeView::customContextMenuRequested, this,
            &LogFileExplorer::show_context_menu);

    connect(m_model, &QAbstractItemModel::dataChanged, this,
            [this](const QModelIndex& top_left, const QModelIndex& bottom_right,
                   const QList<int>& roles) {
                Q_UNUSED(bottom_right);

                // Only react to display/edit role changes
                if (!roles.isEmpty() && !roles.contains(Qt::DisplayRole) &&
                    !roles.contains(Qt::EditRole))
                {
                    return;
                }

                if (top_left.isValid())
                {
                    const auto type = get_item_type(top_left);

                    if (type == LogFileTreeItem::Type::Session)
                    {
                        const QString session_id =
                            top_left.data(LogFileTreeModel::SessionIdRole).toString();
                        const QString new_name = top_left.data(Qt::DisplayRole).toString();

                        if (!session_id.isEmpty() && !new_name.isEmpty())
                        {
                            emit rename_session_requested(session_id, new_name);
                        }
                    }
                }
            });
}

/**
 * @brief Initializes the context menus for different item types.
 */
auto LogFileExplorer::init_context_menu() -> void
{
    // File context menu
    m_file_context_menu = new QMenu(this);

    auto* open_action = new QAction(tr("Open in New View"), m_file_context_menu);
    m_file_context_menu->addAction(open_action);
    connect(open_action, &QAction::triggered, this, [this]() {
        [[maybe_unused]] const bool dispatched = dispatch_current_if_type(
            LogFileTreeItem::Type::File,
            [this](const LogFileInfo& info) { emit open_file_requested(info); });
    });

    auto* add_to_view_action = new QAction(tr("Add to Current View"), m_file_context_menu);
    m_file_context_menu->addAction(add_to_view_action);
    connect(add_to_view_action, &QAction::triggered, this, [this]() {
        [[maybe_unused]] const bool dispatched = dispatch_current_if_type(
            LogFileTreeItem::Type::File,
            [this](const LogFileInfo& info) { emit add_to_current_view_requested(info); });
    });

    auto* remove_file_action = new QAction(tr("Remove"), m_file_context_menu);
    m_file_context_menu->addAction(remove_file_action);
    connect(remove_file_action, &QAction::triggered, this, [this]() {
        [[maybe_unused]] const bool dispatched = dispatch_current_if_type(
            LogFileTreeItem::Type::File,
            [this](const LogFileInfo& info) { emit remove_file_requested(info); });
    });

    // Session context menu
    m_session_context_menu = new QMenu(this);

    auto* rename_session_action = new QAction(tr("Rename Session"), m_session_context_menu);
    m_session_context_menu->addAction(rename_session_action);
    connect(rename_session_action, &QAction::triggered, this, [this]() {
        const QModelIndex index = ui->treeView->currentIndex();
        if (index.isValid() && get_item_type(index) == LogFileTreeItem::Type::Session)
        {
            const QString session_id = index.data(LogFileTreeModel::SessionIdRole).toString();
            const QString current_name = index.data(Qt::DisplayRole).toString();

            bool ok = false;
            const QString new_name =
                QInputDialog::getText(this, tr("Rename Session"), tr("Session name:"),
                                      QLineEdit::Normal, current_name, &ok);
            if (ok && !new_name.isEmpty())
            {
                emit rename_session_requested(session_id, new_name);
            }
        }
    });

    m_session_context_menu->addSeparator();

    auto* close_session_action = new QAction(tr("Close Session"), m_session_context_menu);
    m_session_context_menu->addAction(close_session_action);
    connect(close_session_action, &QAction::triggered, this, [this]() {
        const QModelIndex index = ui->treeView->currentIndex();
        if (index.isValid() && get_item_type(index) == LogFileTreeItem::Type::Session)
        {
            const QString session_id = index.data(LogFileTreeModel::SessionIdRole).toString();
            emit close_session_requested(session_id);
        }
    });

    auto* delete_session_action = new QAction(tr("Delete Session..."), m_session_context_menu);
    m_session_context_menu->addAction(delete_session_action);
    connect(delete_session_action, &QAction::triggered, this, [this]() {
        const QModelIndex index = ui->treeView->currentIndex();
        if (index.isValid() && get_item_type(index) == LogFileTreeItem::Type::Session)
        {
            const QString session_id = index.data(LogFileTreeModel::SessionIdRole).toString();
            const QString session_name = index.data(Qt::DisplayRole).toString();

            const QMessageBox::StandardButton reply = QMessageBox::question(
                this, tr("Delete Session"),
                tr("Are you sure you want to permanently delete the session \"%1\"?\n\n"
                   "This action cannot be undone.")
                    .arg(session_name),
                QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

            if (reply == QMessageBox::Yes)
            {
                emit delete_session_requested(session_id);
            }
        }
    });

    // Group context menu (empty for now, can be extended later)
    m_group_context_menu = new QMenu(this);
}

/**
 * @brief Handles context menu requests on the tree view.
 * @param pos The position where the menu should appear.
 */
auto LogFileExplorer::show_context_menu(const QPoint& pos) -> void
{
    const QModelIndex index = ui->treeView->indexAt(pos);

    if (index.isValid())
    {
        const auto type = get_item_type(index);
        const QPoint global_pos = ui->treeView->viewport()->mapToGlobal(pos);

        if (type == LogFileTreeItem::Type::File)
        {
            m_file_context_menu->exec(global_pos);
        }
        else if (type == LogFileTreeItem::Type::Session)
        {
            m_session_context_menu->exec(global_pos);
        }
        else if (type == LogFileTreeItem::Type::Group)
        {
            if (!m_group_context_menu->isEmpty())
            {
                m_group_context_menu->exec(global_pos);
            }
        }
    }
}

/**
 * @brief Gets the item type at the given index.
 * @param index The model index.
 * @return The item type, or Group if invalid.
 */
auto LogFileExplorer::get_item_type(const QModelIndex& index) const -> LogFileTreeItem::Type
{
    LogFileTreeItem::Type type = LogFileTreeItem::Type::Group;

    if (index.isValid())
    {
        const QVariant type_var = index.data(LogFileTreeModel::ItemTypeRole);
        if (type_var.canConvert<LogFileTreeItem::Type>())
        {
            type = type_var.value<LogFileTreeItem::Type>();
        }
    }

    return type;
}

/**
 * @brief Tries to extract a LogFileInfo for the given index if the item type matches.
 * @param index The model index to inspect.
 * @param expected_type The expected LogFileTreeItem::Type.
 * @param out_info Output parameter that receives the LogFileInfo on success.
 * @return True if successful; false otherwise.
 */
auto LogFileExplorer::try_get_info_if_type(const QModelIndex& index,
                                           LogFileTreeItem::Type expected_type,
                                           LogFileInfo& out_info) const -> bool
{
    bool success = false;

    if (index.isValid() && get_item_type(index) == expected_type)
    {
        if (expected_type == LogFileTreeItem::Type::File)
        {
            auto* item = static_cast<LogFileTreeItem*>(index.internalPointer());
            out_info = item->data(1).value<LogFileInfo>();
            success = true;
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
auto LogFileExplorer::dispatch_current_if_type(
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
