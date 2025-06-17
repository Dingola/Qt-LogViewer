/**
 * @file LogModel.cpp
 * @brief This file contains the implementation of the LogModel class.
 */

#include "Qt-LogViewer/Models/LogModel.h"

/**
 * @brief Constructs a LogModel object.
 * @param parent The parent QObject.
 *
 * Initializes an empty log model for use in Qt's model/view framework.
 */
LogModel::LogModel(QObject* parent): QAbstractTableModel(parent) {}

/**
 * @brief Returns the number of rows in the model.
 *
 * This method returns the number of log entries currently stored in the model.
 * If the parent index is valid (i.e., not the root), 0 is returned, as this is a flat table model.
 *
 * @param parent The parent index (should be invalid for a flat table).
 * @return The number of log entries (rows) in the model.
 */
auto LogModel::rowCount(const QModelIndex& parent) const -> int
{
    int row_count = 0;

    if (!parent.isValid())
    {
        row_count = m_entries.size();
    }

    return row_count;
}

/**
 * @brief Returns the number of columns in the model.
 *
 * This method returns the number of columns, which corresponds to the number of
 * fields in a log entry (timestamp, level, message, app name).
 *
 * @param parent The parent index (unused).
 * @return The number of columns in the model.
 */
auto LogModel::columnCount(const QModelIndex& parent) const -> int
{
    Q_UNUSED(parent);
    return ColumnCount;
}

/**
 * @brief Returns the data for the given index and role.
 *
 * Provides the data to be displayed or edited in the view, or accessed via custom roles.
 * Supports both standard Qt roles (DisplayRole, EditRole) and custom roles for QML or advanced
 * views.
 *
 * @param index The model index specifying the row and column.
 * @param role The data role (DisplayRole, EditRole, or custom LogRole).
 * @return The data value for the given index and role, or an invalid QVariant if not applicable.
 */
auto LogModel::data(const QModelIndex& index, int role) const -> QVariant
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_entries.size())
    {
        return QVariant();
    }

    const LogEntry& entry = m_entries.at(index.row());

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        switch (index.column())
        {
        case Timestamp:
            return entry.get_timestamp();
        case Level:
            return entry.get_level();
        case Message:
            return entry.get_message();
        case AppName:
            return entry.get_app_name();
        default:
            return QVariant();
        }
    }

    switch (role)
    {
    case TimestampRole:
        return entry.get_timestamp();
    case LevelRole:
        return entry.get_level();
    case MessageRole:
        return entry.get_message();
    case AppNameRole:
        return entry.get_app_name();
    default:
        return QVariant();
    }
}

/**
 * @brief Returns the header data for the given section, orientation, and role.
 *
 * Provides the text for the horizontal headers (column names) in the view.
 *
 * @param section The section index (column number).
 * @param orientation The orientation (should be Qt::Horizontal).
 * @param role The data role (should be Qt::DisplayRole).
 * @return The header text for the given section, or an invalid QVariant if not applicable.
 */
auto LogModel::headerData(int section, Qt::Orientation orientation, int role) const -> QVariant
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
    {
        return QVariant();
    }

    switch (section)
    {
    case Timestamp:
        return QStringLiteral("Timestamp");
    case Level:
        return QStringLiteral("Level");
    case Message:
        return QStringLiteral("Message");
    case AppName:
        return QStringLiteral("App Name");
    default:
        return QVariant();
    }
}

/**
 * @brief Returns the item flags for the given index.
 *
 * Specifies the properties of each item in the model (e.g., selectable, enabled).
 * All items are selectable and enabled, but not editable by default.
 *
 * @param index The model index.
 * @return The item flags for the given index.
 */
auto LogModel::flags(const QModelIndex& index) const -> Qt::ItemFlags
{
    Qt::ItemFlags item_flags = Qt::NoItemFlags;

    if (index.isValid())
    {
        item_flags = (Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    }

    return item_flags;
}

/**
 * @brief Returns the role names for the model.
 *
 * Provides a mapping from custom role integers to role names, useful for QML integration.
 *
 * @return A hash mapping role integers to role names.
 */
auto LogModel::roleNames() const -> QHash<int, QByteArray>
{
    QHash<int, QByteArray> roles;
    roles[TimestampRole] = "timestamp";
    roles[LevelRole] = "level";
    roles[MessageRole] = "message";
    roles[AppNameRole] = "app_name";
    return roles;
}

/**
 * @brief Adds a log entry to the model.
 *
 * Appends a new LogEntry to the end of the model and notifies any attached views.
 *
 * @param entry The LogEntry to add.
 */
auto LogModel::add_entry(const LogEntry& entry) -> void
{
    beginInsertRows(QModelIndex(), m_entries.size(), m_entries.size());
    m_entries.append(entry);
    endInsertRows();
}

/**
 * @brief Removes all log entries from the model.
 *
 * Clears the internal list of log entries and resets the model.
 */
auto LogModel::clear() -> void
{
    beginResetModel();
    m_entries.clear();
    endResetModel();
}

/**
 * @brief Returns the log entry at the given row.
 *
 * Retrieves the LogEntry at the specified row index. If the row is out of range,
 * a default-constructed LogEntry is returned.
 *
 * @param row The row index.
 * @return The LogEntry at the specified row, or a default LogEntry if out of range.
 */
auto LogModel::get_entry(int row) const -> LogEntry
{
    if (row < 0 || row >= m_entries.size())
    {
        return LogEntry();
    }

    return m_entries.at(row);
}

/**
 * @brief Returns all log entries.
 *
 * Provides a copy of the internal list of all LogEntry objects in the model.
 *
 * @return A QVector containing all log entries.
 */
auto LogModel::get_entries() const -> QVector<LogEntry>
{
    return m_entries;
}

/**
 * @brief Sets all log entries, replacing the current data.
 *
 * Replaces the current list of log entries with a new list and resets the model.
 *
 * @param entries The new list of log entries.
 */
auto LogModel::set_entries(const QVector<LogEntry>& entries) -> void
{
    beginResetModel();
    m_entries = entries;
    endResetModel();
}
