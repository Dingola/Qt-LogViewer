#include "Qt-LogViewer/Models/LogFileTreeModel.h"

#include "Qt-LogViewer/Models/LogFileInfo.h"
#include "Qt-LogViewer/Models/LogFileTreeItem.h"

/**
 * @brief Constructs a LogFileTreeModel object.
 * @param parent The parent QObject, or nullptr.
 */
LogFileTreeModel::LogFileTreeModel(QObject* parent): QAbstractItemModel(parent)
{
    QVector<QVariant> root_data;
    root_data << tr("Application / File");
    m_root_item = new LogFileTreeItem(root_data);
}

/**
 * @brief Destroys the LogFileTreeModel object and its tree structure.
 */
LogFileTreeModel::~LogFileTreeModel()
{
    delete m_root_item;
}

/**
 * @brief Sets the log files to be displayed and grouped. Rebuilds the entire tree structure.
 * @param log_file_infos The list of LogFileInfo objects.
 */
auto LogFileTreeModel::set_log_files(const QList<LogFileInfo>& log_file_infos) -> void
{
    beginResetModel();
    delete m_root_item;
    m_group_items.clear();
    QVector<QVariant> root_data;
    root_data << QVariant::fromValue(LogFileTreeItem::Type::Group) << tr("Application / File");
    m_root_item = new LogFileTreeItem(root_data);

    const auto grouped = group_by_app_name(log_file_infos);

    for (auto it = grouped.constBegin(); it != grouped.constEnd(); ++it)
    {
        QVector<QVariant> group_data;
        group_data << QVariant::fromValue(LogFileTreeItem::Type::Group) << it.key();
        auto* group_item = new LogFileTreeItem(group_data, m_root_item);

        for (const auto& file_info: it.value())
        {
            QVector<QVariant> file_data;
            file_data << QVariant::fromValue(LogFileTreeItem::Type::File)
                      << QVariant::fromValue(file_info);
            auto* file_item = new LogFileTreeItem(file_data, group_item);
            group_item->append_child(file_item);
        }

        m_root_item->append_child(group_item);
        m_group_items[it.key()] = group_item;
    }

    endResetModel();
}

/**
 * @brief Adds a single log file to the model. Creates a new group if necessary.
 * @param log_file_info The LogFileInfo to add.
 */
auto LogFileTreeModel::add_log_file(const LogFileInfo& log_file_info) -> void
{
    QString app_name = log_file_info.get_app_name();

    if (app_name.isEmpty())
    {
        app_name = tr("Unknown");
    }

    LogFileTreeItem* group_item = m_group_items.value(app_name, nullptr);
    int group_row = -1;

    if (!group_item)
    {
        group_row = m_root_item->child_count();
        beginInsertRows(QModelIndex(), group_row, group_row);
        QVector<QVariant> group_data;
        group_data << QVariant::fromValue(LogFileTreeItem::Type::Group) << app_name;
        group_item = new LogFileTreeItem(group_data, m_root_item);
        m_root_item->append_child(group_item);
        m_group_items[app_name] = group_item;
        endInsertRows();
    }

    int file_row = group_item->child_count();
    int group_index = -1;

    for (int i = 0; i < m_root_item->child_count(); ++i)
    {
        if (m_root_item->child(i) == group_item)
        {
            group_index = i;
        }
    }

    beginInsertRows(index(group_index, 0, QModelIndex()), file_row, file_row);
    QVector<QVariant> file_data;
    file_data << QVariant::fromValue(LogFileTreeItem::Type::File)
              << QVariant::fromValue(log_file_info);
    auto* file_item = new LogFileTreeItem(file_data, group_item);
    group_item->append_child(file_item);
    endInsertRows();
}

/**
 * @brief Removes a single log file from the model. Removes the group if it becomes empty.
 * @param log_file_info The LogFileInfo to remove.
 */
auto LogFileTreeModel::remove_log_file(const LogFileInfo& log_file_info) -> void
{
    QString app_name = log_file_info.get_app_name();

    if (app_name.isEmpty())
    {
        app_name = tr("Unknown");
    }

    LogFileTreeItem* group_item = m_group_items.value(app_name, nullptr);

    if (group_item)
    {
        int group_index = -1;

        for (int i = 0; i < m_root_item->child_count(); ++i)
        {
            if (m_root_item->child(i) == group_item)
            {
                group_index = i;
            }
        }

        int file_row = find_file_row(group_item, log_file_info.get_file_path());

        if (file_row >= 0)
        {
            beginRemoveRows(index(group_index, 0, QModelIndex()), file_row, file_row);
            LogFileTreeItem* file_item = group_item->remove_child(file_row);
            delete file_item;
            endRemoveRows();

            if (group_item->child_count() == 0)
            {
                beginRemoveRows(QModelIndex(), group_index, group_index);
                LogFileTreeItem* removed_group = m_root_item->remove_child(group_index);
                m_group_items.remove(app_name);
                delete removed_group;
                endRemoveRows();
            }
        }
    }
}

/**
 * @brief Returns the number of columns for the given parent.
 * @param parent The parent index.
 * @return The number of columns.
 */
auto LogFileTreeModel::columnCount(const QModelIndex& parent) const -> int
{
    int count = m_root_item->column_count();

    if (parent.isValid())
    {
        auto* item = static_cast<LogFileTreeItem*>(parent.internalPointer());
        count = item->column_count();
    }

    return count;
}

/**
 * @brief Returns the number of rows under the given parent.
 * @param parent The parent index.
 * @return The number of child rows.
 */
auto LogFileTreeModel::rowCount(const QModelIndex& parent) const -> int
{
    LogFileTreeItem* parent_item = m_root_item;

    if (parent.isValid())
    {
        parent_item = static_cast<LogFileTreeItem*>(parent.internalPointer());
    }

    return parent_item->child_count();
}

/**
 * @brief Returns the data for the given index and role.
 * @param index The model index.
 * @param role The data role.
 * @return The data value for the given index and role.
 */
auto LogFileTreeModel::data(const QModelIndex& index, int role) const -> QVariant
{
    QVariant value;

    if (index.isValid())
    {
        auto* item = static_cast<LogFileTreeItem*>(index.internalPointer());
        auto type = item->data(0).value<LogFileTreeItem::Type>();

        if (role == Qt::DisplayRole || role == Qt::EditRole)
        {
            if (type == LogFileTreeItem::Type::Group)
            {
                if (index.column() == 0)
                {
                    value = item->data(1);
                }
            }
            else if (type == LogFileTreeItem::Type::File)
            {
                LogFileInfo log_file_info = item->data(1).value<LogFileInfo>();
                switch (index.column())
                {
                case 0:
                    value = log_file_info.get_file_name();
                    break;
                case 1:
                    value = log_file_info.get_file_path();
                    break;
                case 2:
                    value = log_file_info.get_app_name();
                    break;
                default:
                    break;
                }
            }
        }
    }

    return value;
}

/**
 * @brief Returns the item flags for the given index.
 * @param index The model index.
 * @return The item flags.
 */
auto LogFileTreeModel::flags(const QModelIndex& index) const -> Qt::ItemFlags
{
    Qt::ItemFlags flags = Qt::NoItemFlags;

    if (index.isValid())
    {
        flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    }

    return flags;
}

/**
 * @brief Returns the header data for the given section, orientation, and role.
 * @param section The section index.
 * @param orientation The orientation.
 * @param role The data role.
 * @return The header text for the given section.
 */
auto LogFileTreeModel::headerData(int section, Qt::Orientation orientation,
                                  int role) const -> QVariant
{
    QVariant header;

    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        header = m_root_item->data(section);
    }

    return header;
}

/**
 * @brief Returns the index for the given row, column, and parent.
 * @param row The row index.
 * @param column The column index.
 * @param parent The parent index.
 * @return The model index.
 */
auto LogFileTreeModel::index(int row, int column, const QModelIndex& parent) const -> QModelIndex
{
    LogFileTreeItem* parent_item = m_root_item;

    if (parent.isValid())
    {
        parent_item = static_cast<LogFileTreeItem*>(parent.internalPointer());
    }

    LogFileTreeItem* child_item = parent_item->child(row);
    QModelIndex idx;

    if (child_item)
    {
        idx = createIndex(row, column, child_item);
    }

    return idx;
}

/**
 * @brief Returns the parent index for the given index.
 * @param index The model index.
 * @return The parent index.
 */
auto LogFileTreeModel::parent(const QModelIndex& index) const -> QModelIndex
{
    QModelIndex parent_idx;

    if (index.isValid())
    {
        auto* child_item = static_cast<LogFileTreeItem*>(index.internalPointer());
        LogFileTreeItem* parent_item = child_item->parent_item();

        if (parent_item && parent_item != m_root_item)
        {
            parent_idx = createIndex(parent_item->row(), 0, parent_item);
        }
    }

    return parent_idx;
}

/**
 * @brief Helper to group files by application name.
 * @param files The list of LogFileInfo objects.
 * @return A map from app name to list of files.
 */
auto LogFileTreeModel::group_by_app_name(const QList<LogFileInfo>& files)
    -> QMap<QString, QList<LogFileInfo>>
{
    QMap<QString, QList<LogFileInfo>> groups;

    for (int i = 0; i < files.size(); ++i)
    {
        QString app_name = files.at(i).get_app_name();

        if (app_name.isEmpty())
        {
            app_name = tr("Unknown");
        }

        if (!groups.contains(app_name))
        {
            groups[app_name] = QList<LogFileInfo>();
        }

        groups[app_name].append(files.at(i));
    }

    return groups;
}

/**
 * @brief Finds the row of a file in a group by file name.
 * @param group_item The group item.
 * @param file_path The file path to search for.
 * @return The row index, or -1 if not found.
 */
auto LogFileTreeModel::find_file_row(LogFileTreeItem* group_item,
                                     const QString& file_path) const -> int
{
    int found_row = -1;
    LogFileInfo info;

    for (int i = 0; i < group_item->child_count(); ++i)
    {
        auto* child = group_item->child(i);
        auto type = child->data(0).value<LogFileTreeItem::Type>();
        if (type == LogFileTreeItem::Type::File)
        {
            info = child->data(1).value<LogFileInfo>();

            if (found_row == -1 && info.get_file_path() == file_path)
            {
                found_row = i;
            }
        }
    }

    return found_row;
}
