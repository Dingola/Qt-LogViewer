#include "Qt-LogViewer/Models/LogFileTreeModel.h"

#include <QIcon>

#include "Qt-LogViewer/Models/LogFileInfo.h"
#include "Qt-LogViewer/Models/LogFileTreeItem.h"
#include "Qt-LogViewer/Services/UiUtils.h"

/**
 * @brief Constructs a LogFileTreeModel object.
 * @param parent The parent QObject, or nullptr.
 */
LogFileTreeModel::LogFileTreeModel(QObject* parent): QAbstractItemModel(parent)
{
    QVector<QVariant> root_data;
    root_data << QVariant::fromValue(LogFileTreeItem::Type::Group) << tr("Sessions");
    m_root_item = new LogFileTreeItem(root_data);
}

/**
 * @brief Destroys the LogFileTreeModel object and its tree structure.
 */
LogFileTreeModel::~LogFileTreeModel()
{
    delete m_root_item;
}

// -----------------------------------------------------------------------------
// Session management
// -----------------------------------------------------------------------------

/**
 * @brief Adds a new session to the tree.
 * @param session_id Unique identifier for the session.
 * @param session_name Display name for the session.
 * @return True if the session was added; false if it already exists.
 */
auto LogFileTreeModel::add_session(const QString& session_id, const QString& session_name) -> bool
{
    bool added = false;

    if (!session_id.isEmpty() && !m_session_items.contains(session_id))
    {
        const int row = m_root_item->child_count();
        beginInsertRows(QModelIndex(), row, row);

        QVector<QVariant> session_data;
        session_data << QVariant::fromValue(LogFileTreeItem::Type::Session) << session_id
                     << session_name;
        auto* session_item = new LogFileTreeItem(session_data, m_root_item);
        m_root_item->append_child(session_item);
        m_session_items[session_id] = session_item;

        endInsertRows();
        added = true;
    }

    return added;
}

/**
 * @brief Removes a session and all its children from the tree.
 * @param session_id The session identifier.
 * @return True if the session was removed; false if not found.
 */
auto LogFileTreeModel::remove_session(const QString& session_id) -> bool
{
    bool removed = false;
    LogFileTreeItem* session_item = m_session_items.value(session_id, nullptr);

    if (session_item != nullptr)
    {
        const int row = get_session_row(session_item);

        if (row >= 0)
        {
            beginRemoveRows(QModelIndex(), row, row);

            QList<GroupKey> keys_to_remove;
            for (auto it = m_group_items.constBegin(); it != m_group_items.constEnd(); ++it)
            {
                if (it.key().session_id == session_id)
                {
                    keys_to_remove.append(it.key());
                }
            }
            for (const auto& key: keys_to_remove)
            {
                m_group_items.remove(key);
            }

            LogFileTreeItem* removed_item = m_root_item->remove_child(row);
            m_session_items.remove(session_id);
            delete removed_item;

            endRemoveRows();
            removed = true;

            if (m_session_items.isEmpty())
            {
                emit all_sessions_removed();
            }
        }
    }

    return removed;
}

/**
 * @brief Renames an existing session.
 * @param session_id The session identifier.
 * @param new_name The new display name.
 * @return True if renamed; false if session not found.
 */
auto LogFileTreeModel::rename_session(const QString& session_id, const QString& new_name) -> bool
{
    bool renamed = false;
    LogFileTreeItem* session_item = m_session_items.value(session_id, nullptr);

    if (session_item != nullptr)
    {
        session_item->set_data(2, new_name);
        const int row = get_session_row(session_item);
        QModelIndex idx = index(row, 0, QModelIndex());
        emit dataChanged(idx, idx, {Qt::DisplayRole, Qt::EditRole});
        renamed = true;
    }

    return renamed;
}

/**
 * @brief Checks if a session exists.
 * @param session_id The session identifier.
 * @return True if the session exists.
 */
auto LogFileTreeModel::has_session(const QString& session_id) const -> bool
{
    return m_session_items.contains(session_id);
}

/**
 * @brief Returns the number of sessions in the model.
 * @return The session count.
 */
auto LogFileTreeModel::get_session_count() const -> int
{
    return m_session_items.size();
}

/**
 * @brief Returns the model index for a session.
 * @param session_id The session identifier.
 * @return The model index, or invalid if not found.
 */
auto LogFileTreeModel::get_session_index(const QString& session_id) const -> QModelIndex
{
    QModelIndex result;
    LogFileTreeItem* session_item = m_session_items.value(session_id, nullptr);

    if (session_item != nullptr)
    {
        const int row = get_session_row(session_item);
        if (row >= 0)
        {
            result = createIndex(row, 0, session_item);
        }
    }

    return result;
}

// -----------------------------------------------------------------------------
// Log file management (session-aware)
// -----------------------------------------------------------------------------

/**
 * @brief Sets the log files for a specific session (replaces existing files).
 * @param session_id The session identifier.
 * @param log_file_infos The list of LogFileInfo objects.
 */
auto LogFileTreeModel::set_log_files(const QString& session_id,
                                     const QList<LogFileInfo>& log_file_infos) -> void
{
    LogFileTreeItem* session_item = m_session_items.value(session_id, nullptr);

    if (session_item == nullptr)
    {
        add_session(session_id, session_id);
        session_item = m_session_items.value(session_id);
    }

    const int session_row = get_session_row(session_item);
    const QModelIndex session_index = index(session_row, 0, QModelIndex());

    if (session_item->child_count() > 0)
    {
        beginRemoveRows(session_index, 0, session_item->child_count() - 1);

        QList<GroupKey> keys_to_remove;
        for (auto it = m_group_items.constBegin(); it != m_group_items.constEnd(); ++it)
        {
            if (it.key().session_id == session_id)
            {
                keys_to_remove.append(it.key());
            }
        }
        for (const auto& key: keys_to_remove)
        {
            m_group_items.remove(key);
        }

        while (session_item->child_count() > 0)
        {
            LogFileTreeItem* child = session_item->remove_child(0);
            delete child;
        }

        endRemoveRows();
    }

    const auto grouped = group_by_app_name(log_file_infos);

    for (auto it = grouped.constBegin(); it != grouped.constEnd(); ++it)
    {
        const int group_row = session_item->child_count();
        beginInsertRows(session_index, group_row, group_row);

        QVector<QVariant> group_data;
        group_data << QVariant::fromValue(LogFileTreeItem::Type::Group) << it.key();
        auto* group_item = new LogFileTreeItem(group_data, session_item);
        session_item->append_child(group_item);
        m_group_items[{session_id, it.key()}] = group_item;

        endInsertRows();

        const QModelIndex group_index = index(group_row, 0, session_index);

        beginInsertRows(group_index, 0, it.value().size() - 1);
        for (const auto& file_info: it.value())
        {
            QVector<QVariant> file_data;
            file_data << QVariant::fromValue(LogFileTreeItem::Type::File)
                      << QVariant::fromValue(file_info);
            auto* file_item = new LogFileTreeItem(file_data, group_item);
            group_item->append_child(file_item);
        }
        endInsertRows();
    }
}

/**
 * @brief Adds a single log file to a session.
 * @param session_id The session identifier.
 * @param log_file_info The LogFileInfo to add.
 * @return True if added; false if session not found or file already exists.
 */
auto LogFileTreeModel::add_log_file(const QString& session_id,
                                    const LogFileInfo& log_file_info) -> bool
{
    bool added = false;
    LogFileTreeItem* session_item = m_session_items.value(session_id, nullptr);

    if (session_item == nullptr)
    {
        add_session(session_id, session_id);
        session_item = m_session_items.value(session_id);
    }

    QString app_name = log_file_info.get_app_name();
    if (app_name.isEmpty())
    {
        app_name = tr("Unknown");
    }

    LogFileTreeItem* group_item = find_or_create_group(session_item, session_id, app_name);

    const int existing_row = find_file_row(group_item, log_file_info.get_file_path());
    if (existing_row < 0)
    {
        const int session_row = get_session_row(session_item);
        const QModelIndex session_index = index(session_row, 0, QModelIndex());

        int group_row = -1;
        for (int i = 0; i < session_item->child_count(); ++i)
        {
            if (session_item->child(i) == group_item)
            {
                group_row = i;
            }
        }

        const QModelIndex group_index = index(group_row, 0, session_index);
        const int file_row = group_item->child_count();

        beginInsertRows(group_index, file_row, file_row);

        QVector<QVariant> file_data;
        file_data << QVariant::fromValue(LogFileTreeItem::Type::File)
                  << QVariant::fromValue(log_file_info);
        auto* file_item = new LogFileTreeItem(file_data, group_item);
        group_item->append_child(file_item);

        endInsertRows();
        added = true;
    }

    return added;
}

/**
 * @brief Adds multiple log files to a session.
 * @param session_id The session identifier.
 * @param log_file_infos The list of LogFileInfo objects to add.
 * @return Number of files successfully added.
 */
auto LogFileTreeModel::add_log_files(const QString& session_id,
                                     const QList<LogFileInfo>& log_file_infos) -> int
{
    int count = 0;

    for (const auto& file_info: log_file_infos)
    {
        if (add_log_file(session_id, file_info))
        {
            ++count;
        }
    }

    return count;
}

/**
 * @brief Removes a single log file from a session.
 * @param session_id The session identifier.
 * @param log_file_info The LogFileInfo to remove.
 * @return True if removed; false if not found.
 */
auto LogFileTreeModel::remove_log_file(const QString& session_id,
                                       const LogFileInfo& log_file_info) -> bool
{
    bool removed = false;
    LogFileTreeItem* session_item = m_session_items.value(session_id, nullptr);

    if (session_item != nullptr)
    {
        QString app_name = log_file_info.get_app_name();
        if (app_name.isEmpty())
        {
            app_name = tr("Unknown");
        }

        LogFileTreeItem* group_item = m_group_items.value({session_id, app_name}, nullptr);

        if (group_item != nullptr)
        {
            const int file_row = find_file_row(group_item, log_file_info.get_file_path());

            if (file_row >= 0)
            {
                const int session_row = get_session_row(session_item);
                const QModelIndex session_index = index(session_row, 0, QModelIndex());

                int group_row = -1;
                for (int i = 0; i < session_item->child_count(); ++i)
                {
                    if (session_item->child(i) == group_item)
                    {
                        group_row = i;
                    }
                }

                const QModelIndex group_index = index(group_row, 0, session_index);

                beginRemoveRows(group_index, file_row, file_row);
                LogFileTreeItem* file_item = group_item->remove_child(file_row);
                delete file_item;
                endRemoveRows();

                removed = true;

                if (group_item->child_count() == 0)
                {
                    beginRemoveRows(session_index, group_row, group_row);
                    LogFileTreeItem* removed_group = session_item->remove_child(group_row);
                    m_group_items.remove({session_id, app_name});
                    delete removed_group;
                    endRemoveRows();
                }
            }
        }
    }

    return removed;
}

/**
 * @brief Clears all log files from a session (keeps the session node).
 * @param session_id The session identifier.
 */
auto LogFileTreeModel::clear_session_files(const QString& session_id) -> void
{
    LogFileTreeItem* session_item = m_session_items.value(session_id, nullptr);

    if (session_item != nullptr && session_item->child_count() > 0)
    {
        const int session_row = get_session_row(session_item);
        const QModelIndex session_index = index(session_row, 0, QModelIndex());

        beginRemoveRows(session_index, 0, session_item->child_count() - 1);

        QList<GroupKey> keys_to_remove;
        for (auto it = m_group_items.constBegin(); it != m_group_items.constEnd(); ++it)
        {
            if (it.key().session_id == session_id)
            {
                keys_to_remove.append(it.key());
            }
        }
        for (const auto& key: keys_to_remove)
        {
            m_group_items.remove(key);
        }

        while (session_item->child_count() > 0)
        {
            LogFileTreeItem* child = session_item->remove_child(0);
            delete child;
        }

        endRemoveRows();
    }
}

// -----------------------------------------------------------------------------
// Log file management (all sessions)
// -----------------------------------------------------------------------------

/**
 * @brief Sets the log files for all existing sessions (replaces existing files).
 * @param log_file_infos The list of LogFileInfo objects.
 */
auto LogFileTreeModel::set_log_files(const QList<LogFileInfo>& log_file_infos) -> void
{
    const QList<QString> session_ids = m_session_items.keys();
    for (const QString& session_id: session_ids)
    {
        set_log_files(session_id, log_file_infos);
    }
}

/**
 * @brief Adds a single log file to all existing sessions.
 * @param log_file_info The LogFileInfo to add.
 */
auto LogFileTreeModel::add_log_file(const LogFileInfo& log_file_info) -> void
{
    const QList<QString> session_ids = m_session_items.keys();
    for (const QString& session_id: session_ids)
    {
        add_log_file(session_id, log_file_info);
    }
}

/**
 * @brief Removes a single log file from all sessions where it exists.
 * @param log_file_info The LogFileInfo to remove.
 */
auto LogFileTreeModel::remove_log_file(const LogFileInfo& log_file_info) -> void
{
    const QList<QString> session_ids = m_session_items.keys();
    for (const QString& session_id: session_ids)
    {
        remove_log_file(session_id, log_file_info);
    }
}

// -----------------------------------------------------------------------------
// QAbstractItemModel overrides
// -----------------------------------------------------------------------------

/**
 * @brief Returns the number of columns for the given parent.
 * @param parent The parent index.
 * @return The number of columns.
 */
auto LogFileTreeModel::columnCount(const QModelIndex& parent) const -> int
{
    Q_UNUSED(parent);
    return ColumnCount;
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
        const auto type = item->data(0).value<LogFileTreeItem::Type>();

        if (role == Qt::DisplayRole || role == Qt::EditRole)
        {
            if (type == LogFileTreeItem::Type::Session)
            {
                value = item->data(2);  // session_name
            }
            else if (type == LogFileTreeItem::Type::Group)
            {
                value = item->data(1);  // app_name
            }
            else if (type == LogFileTreeItem::Type::File)
            {
                const auto log_file_info = item->data(1).value<LogFileInfo>();
                value = log_file_info.get_file_name();
            }
        }
        else if (role == Qt::DecorationRole && index.column() == 0)
        {
            if (type == LogFileTreeItem::Type::Session)
            {
                value = QIcon(
                    UiUtils::colored_svg_icon(":/Resources/Icons/session.svg", QColor("#ffa726")));
            }
            else if (type == LogFileTreeItem::Type::Group)
            {
                value = QIcon(
                    UiUtils::colored_svg_icon(":/Resources/Icons/folder.svg", QColor("#66bb6a")));
            }
            else if (type == LogFileTreeItem::Type::File)
            {
                value = QIcon(
                    UiUtils::colored_svg_icon(":/Resources/Icons/file.svg", QColor("#42a5f5")));
            }
        }
        else if (role == ItemTypeRole)
        {
            value = QVariant::fromValue(type);
        }
        else if (role == SessionIdRole)
        {
            if (type == LogFileTreeItem::Type::Session)
            {
                value = item->data(1);
            }
            else
            {
                LogFileTreeItem* current = item->parent_item();
                while (current != nullptr && current != m_root_item)
                {
                    const auto parent_type = current->data(0).value<LogFileTreeItem::Type>();
                    if (parent_type == LogFileTreeItem::Type::Session)
                    {
                        value = current->data(1);
                        current = nullptr;
                    }
                    else
                    {
                        current = current->parent_item();
                    }
                }
            }
        }
        else if (role == FilePathRole && type == LogFileTreeItem::Type::File)
        {
            const auto log_file_info = item->data(1).value<LogFileInfo>();
            value = log_file_info.get_file_path();
        }
        else if (role == AppNameRole)
        {
            if (type == LogFileTreeItem::Type::Group)
            {
                value = item->data(1);
            }
            else if (type == LogFileTreeItem::Type::File)
            {
                const auto log_file_info = item->data(1).value<LogFileInfo>();
                value = log_file_info.get_app_name();
            }
        }
    }

    return value;
}

/**
 * @brief Sets the data for the given index and role.
 * @param index The model index.
 * @param value The value to set.
 * @param role The data role.
 * @return True if the data was set successfully; false otherwise.
 */
auto LogFileTreeModel::setData(const QModelIndex& index, const QVariant& value, int role) -> bool
{
    bool success = false;

    if (index.isValid() && role == Qt::EditRole)
    {
        auto* item = static_cast<LogFileTreeItem*>(index.internalPointer());
        const auto type = item->data(0).value<LogFileTreeItem::Type>();

        if (type == LogFileTreeItem::Type::Session)
        {
            item->set_data(2, value);
            emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
            success = true;
        }
    }

    return success;
}

/**
 * @brief Returns the item flags for the given index.
 * @param index The model index.
 * @return The item flags.
 */
auto LogFileTreeModel::flags(const QModelIndex& index) const -> Qt::ItemFlags
{
    Qt::ItemFlags item_flags = Qt::NoItemFlags;

    if (index.isValid())
    {
        item_flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

        auto* item = static_cast<LogFileTreeItem*>(index.internalPointer());
        const auto type = item->data(0).value<LogFileTreeItem::Type>();

        if (type == LogFileTreeItem::Type::Session)
        {
            item_flags |= Qt::ItemIsEditable;
        }
    }

    return item_flags;
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

    if (role == Qt::DisplayRole && orientation == Qt::Horizontal && section == 0)
    {
        header = tr("Name");
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

    if (child_item != nullptr)
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

        if (parent_item != nullptr && parent_item != m_root_item)
        {
            parent_idx = createIndex(parent_item->row(), 0, parent_item);
        }
    }

    return parent_idx;
}

// -----------------------------------------------------------------------------
// Private helpers
// -----------------------------------------------------------------------------

/**
 * @brief Helper to group files by application name.
 * @param files The list of LogFileInfo objects.
 * @return A map from app name to list of files.
 */
auto LogFileTreeModel::group_by_app_name(const QList<LogFileInfo>& files)
    -> QMap<QString, QList<LogFileInfo>>
{
    QMap<QString, QList<LogFileInfo>> groups;

    for (const auto& file: files)
    {
        QString app_name = file.get_app_name();
        if (app_name.isEmpty())
        {
            app_name = tr("Unknown");
        }
        groups[app_name].append(file);
    }

    return groups;
}

/**
 * @brief Finds the row of a file in a group by file path.
 * @param group_item The group item.
 * @param file_path The file path to search for.
 * @return The row index, or -1 if not found.
 */
auto LogFileTreeModel::find_file_row(LogFileTreeItem* group_item,
                                     const QString& file_path) const -> int
{
    int found_row = -1;

    for (int i = 0; i < group_item->child_count() && found_row < 0; ++i)
    {
        auto* child = group_item->child(i);
        const auto type = child->data(0).value<LogFileTreeItem::Type>();

        if (type == LogFileTreeItem::Type::File)
        {
            const auto info = child->data(1).value<LogFileInfo>();
            if (info.get_file_path() == file_path)
            {
                found_row = i;
            }
        }
    }

    return found_row;
}

/**
 * @brief Finds or creates an application group under a session.
 * @param session_item The session item.
 * @param session_id The session identifier (for caching).
 * @param app_name The application name.
 * @return Pointer to the group item.
 */
auto LogFileTreeModel::find_or_create_group(LogFileTreeItem* session_item,
                                            const QString& session_id,
                                            const QString& app_name) -> LogFileTreeItem*
{
    LogFileTreeItem* group_item = m_group_items.value({session_id, app_name}, nullptr);

    if (group_item == nullptr)
    {
        const int session_row = get_session_row(session_item);
        const QModelIndex session_index = index(session_row, 0, QModelIndex());
        const int group_row = session_item->child_count();

        beginInsertRows(session_index, group_row, group_row);

        QVector<QVariant> group_data;
        group_data << QVariant::fromValue(LogFileTreeItem::Type::Group) << app_name;
        group_item = new LogFileTreeItem(group_data, session_item);
        session_item->append_child(group_item);
        m_group_items[{session_id, app_name}] = group_item;

        endInsertRows();
    }

    return group_item;
}

/**
 * @brief Removes empty groups from a session.
 * @param session_item The session item to clean up.
 * @param session_id The session identifier.
 */
auto LogFileTreeModel::remove_empty_groups(LogFileTreeItem* session_item,
                                           const QString& session_id) -> void
{
    const int session_row = get_session_row(session_item);
    const QModelIndex session_index = index(session_row, 0, QModelIndex());

    for (int i = session_item->child_count() - 1; i >= 0; --i)
    {
        LogFileTreeItem* group_item = session_item->child(i);

        if (group_item->child_count() == 0)
        {
            const QString app_name = group_item->data(1).toString();

            beginRemoveRows(session_index, i, i);
            LogFileTreeItem* removed = session_item->remove_child(i);
            m_group_items.remove({session_id, app_name});
            delete removed;
            endRemoveRows();
        }
    }
}

/**
 * @brief Gets the row index of a session item.
 * @param session_item The session item.
 * @return The row index.
 */
auto LogFileTreeModel::get_session_row(LogFileTreeItem* session_item) const -> int
{
    int row = -1;

    for (int i = 0; i < m_root_item->child_count() && row < 0; ++i)
    {
        if (m_root_item->child(i) == session_item)
        {
            row = i;
        }
    }

    return row;
}
