/**
 * @file LogModel.cpp
 * @brief This file contains the implementation of the LogModel class.
 */

#include "Qt-LogViewer/Models/LogModel.h"

#include <QBrush>
#include <QColor>

/**
 * @brief Constructs a LogModel object.
 * @param parent The parent QObject.
 *
 * Initializes an empty log model for use in Qt's model/view framework.
 */
LogModel::LogModel(QObject* parent): QAbstractTableModel(parent)
{
    m_columns = {
        {LogField::Timestamp, QStringLiteral("Timestamp"), LogFieldValueType::Timestamp, false,
         false, true, true},
        {LogField::Level, QStringLiteral("Level"), LogFieldValueType::Text, true, true, true, true},
        {LogField::Message, QStringLiteral("Message"), LogFieldValueType::Text, true, false, true,
         true},
        {LogField::AppName, QStringLiteral("App Name"), LogFieldValueType::Text, true, true, true,
         true},
        {QString(), QString(), LogFieldValueType::Text, false, false, false, true}};
}

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
    return m_columns.size();
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
        return {};
    }

    const LogEntry& entry = m_entries.at(index.row());

    const QString field_id = get_column_field_id(index.column());

    if (role == Qt::ForegroundRole && field_id == LogField::Level)
    {
        switch (map_log_level(entry.get_level()))
        {
        case SimpleCppLogger::LogLevel::Info: {
            return QBrush(QColor("#42a5f5"));
        }
        case SimpleCppLogger::LogLevel::Debug: {
            return QBrush(QColor("#66bb6a"));
        }
        case SimpleCppLogger::LogLevel::Trace: {
            return QBrush(QColor("#b0bec5"));
        }
        case SimpleCppLogger::LogLevel::Warning: {
            return QBrush(QColor("#ffb300"));
        }
        case SimpleCppLogger::LogLevel::Error: {
            return QBrush(QColor("#ef5350"));
        }
        case SimpleCppLogger::LogLevel::Fatal: {
            return QBrush(QColor("#ff1744"));
        }
        default: {
            break;
        }
        }
    }

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        QVariant value = entry.get_parsed_field(field_id);

        if (!value.isValid() && field_id == LogField::Timestamp)
        {
            value = entry.get_timestamp();
        }
        else if (!value.isValid() && field_id == LogField::Level)
        {
            value = entry.get_level();
        }
        else if (!value.isValid() && field_id == LogField::Message)
        {
            value = entry.get_message();
        }
        else if (!value.isValid() && field_id == LogField::AppName)
        {
            value = entry.get_app_name();
        }

        return value;
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
        return {};
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
        return {};
    }

    QVariant header;

    if (section >= 0 && section < m_columns.size())
    {
        header = m_columns.at(section).display_name;
    }

    return header;
}

/**
 * @brief Replaces the ordered table-column schema.
 * @param columns Field definitions used for table data and headers.
 */
auto LogModel::set_columns(const QVector<LogFieldDefinition>& columns) -> void
{
    if (!columns.isEmpty())
    {
        beginResetModel();
        m_columns = columns;
        endResetModel();
    }
}

/**
 * @brief Adds fields not yet represented by the table schema.
 * @param columns Ordered field definitions to merge.
 */
auto LogModel::append_columns(const QVector<LogFieldDefinition>& columns) -> void
{
    QVector<LogFieldDefinition> missing_columns;

    for (const LogFieldDefinition& column: columns)
    {
        if (find_column(column.id) < 0)
        {
            missing_columns.append(column);
        }
    }

    if (!missing_columns.isEmpty())
    {
        const int first_column = m_columns.size();
        const int last_column = first_column + missing_columns.size() - 1;
        beginInsertColumns(QModelIndex(), first_column, last_column);
        m_columns += missing_columns;
        endInsertColumns();
    }
}

/**
 * @brief Returns the stable field identifier for a table column.
 * @param column Zero-based model column.
 * @return Field identifier, or an empty string when the column is invalid.
 */
auto LogModel::get_column_field_id(int column) const -> QString
{
    QString field_id;

    if (column >= 0 && column < m_columns.size())
    {
        field_id = m_columns.at(column).id;
    }

    return field_id;
}

/**
 * @brief Finds the table column for a stable field identifier.
 * @param field_id Stable parser or built-in field identifier.
 * @return Zero-based column, or -1 when absent.
 */
auto LogModel::find_column(const QString& field_id) const -> int
{
    int result = -1;

    for (int index = 0; index < m_columns.size() && result < 0; ++index)
    {
        if (!field_id.isEmpty() && m_columns.at(index).id == field_id)
        {
            result = index;
        }
    }

    return result;
}

/**
 * @brief Returns whether a column supports database sorting.
 * @param column Zero-based model column.
 * @return True when the field is sortable.
 */
auto LogModel::is_column_sortable(int column) const -> bool
{
    return column >= 0 && column < m_columns.size() && m_columns.at(column).sortable;
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
        return {};
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
 * @brief Adds multiple log entries to the model.
 *
 * Inserts a list of LogEntry objects into the model, notifying any attached views
 * about the new rows.
 *
 * @param entries The list of LogEntry objects to add.
 */
auto LogModel::add_entries(const QVector<LogEntry>& entries) -> void
{
    if (!entries.isEmpty())
    {
        beginInsertRows(QModelIndex(), m_entries.size(), m_entries.size() + entries.size() - 1);
        m_entries += entries;
        endInsertRows();
    }
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

/**
 * @brief Removes all log entries associated with the given file path.
 * @param file_path The file path whose log entries should be removed.
 */
auto LogModel::remove_entries_by_file_path(const QString& file_path) -> void
{
    beginResetModel();
    m_entries.erase(std::remove_if(m_entries.begin(), m_entries.end(),
                                   [&file_path](const LogEntry& entry) {
                                       return entry.get_file_info().get_file_path() == file_path;
                                   }),
                    m_entries.end());
    endResetModel();
}

/**
 * @brief Maps a log level string to the corresponding SimpleCppLogger::LogLevel.
 *        Handles various spellings, cases, and substrings (e.g. "critical", "trace_info").
 * @param level_str The log level as string.
 * @return The mapped LogLevel enum value.
 */
auto LogModel::map_log_level(const QString& level_str) -> SimpleCppLogger::LogLevel
{
    QString lvl = level_str.trimmed().toLower();

    if (lvl.contains("fatal"))
    {
        return SimpleCppLogger::LogLevel::Fatal;
    }
    if (lvl.contains("critical"))
    {
        return SimpleCppLogger::LogLevel::Error;
    }
    if (lvl.contains("error") || lvl.contains("err"))
    {
        return SimpleCppLogger::LogLevel::Error;
    }
    if (lvl.contains("warn"))
    {
        return SimpleCppLogger::LogLevel::Warning;
    }
    if (lvl.contains("debug"))
    {
        return SimpleCppLogger::LogLevel::Debug;
    }
    if (lvl.contains("trace"))
    {
        return SimpleCppLogger::LogLevel::Trace;
    }
    if (lvl.contains("info"))
    {
        return SimpleCppLogger::LogLevel::Info;
    }

    return SimpleCppLogger::LogLevel::Info;
}
