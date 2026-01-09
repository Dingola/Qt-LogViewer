/**
 * @file RecentLogFilesModel.cpp
 * @brief Implements the RecentLogFilesModel used to display recent log files.
 */

#include "Qt-LogViewer/Models/RecentLogFilesModel.h"

#include <QFileInfo>

/**
 * @brief Constructs an empty RecentLogFilesModel.
 * @param parent Optional QObject parent.
 */
RecentLogFilesModel::RecentLogFilesModel(QObject* parent): QAbstractTableModel(parent), m_items() {}

/**
 * @brief Returns the number of rows.
 * @param parent The parent index (unused for a flat model).
 * @return Number of items in the model.
 */
auto RecentLogFilesModel::rowCount(const QModelIndex& parent) const -> int
{
    int count = 0;

    if (!parent.isValid())
    {
        count = m_items.size();
    }

    return count;
}

/**
 * @brief Returns the number of columns.
 * @param parent The parent index (unused).
 * @return Number of columns.
 */
auto RecentLogFilesModel::columnCount(const QModelIndex& parent) const -> int
{
    Q_UNUSED(parent);
    return ColumnCount;
}

/**
 * @brief Returns data for the given index/role.
 * @param index Model index.
 * @param role Qt role or custom role.
 * @return Data as QVariant; invalid if out of range or role not supported.
 */
auto RecentLogFilesModel::data(const QModelIndex& index, int role) const -> QVariant
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size())
    {
        return {};
    }

    const RecentLogFileRecord& rec = m_items.at(index.row());

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        switch (index.column())
        {
        case FilePath:
            return rec.file_path;
        case FileName: {
            const QString file_name = extract_file_name(rec.file_path);
            return file_name;
        }
        case AppName:
            return rec.app_name;
        case LastOpened:
            return rec.last_opened;
        default:
            return {};
        }
    }

    switch (role)
    {
    case FilePathRole:
        return rec.file_path;
    case FileNameRole: {
        const QString file_name = extract_file_name(rec.file_path);
        return file_name;
    }
    case AppNameRole:
        return rec.app_name;
    case LastOpenedRole:
        return rec.last_opened;
    default:
        return {};
    }
}

/**
 * @brief Returns header data for columns.
 * @param section Column index.
 * @param orientation Qt::Horizontal expected.
 * @param role Qt role.
 * @return Header text or invalid QVariant.
 */
auto RecentLogFilesModel::headerData(int section, Qt::Orientation orientation,
                                     int role) const -> QVariant
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
    {
        return {};
    }

    switch (section)
    {
    case FilePath:
        return QStringLiteral("File Path");
    case FileName:
        return QStringLiteral("File Name");
    case AppName:
        return QStringLiteral("App");
    case LastOpened:
        return QStringLiteral("Last Opened");
    default:
        return {};
    }
}

/**
 * @brief Returns item flags.
 * @param index The model index.
 * @return Selectable + enabled for valid items.
 */
auto RecentLogFilesModel::flags(const QModelIndex& index) const -> Qt::ItemFlags
{
    Qt::ItemFlags item_flags = Qt::NoItemFlags;

    if (index.isValid())
    {
        item_flags = (Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    }

    return item_flags;
}

/**
 * @brief Returns role names for custom roles.
 * @return Mapping of role ids to names.
 */
auto RecentLogFilesModel::roleNames() const -> QHash<int, QByteArray>
{
    QHash<int, QByteArray> roles;
    roles[FilePathRole] = "file_path";
    roles[FileNameRole] = "file_name";
    roles[AppNameRole] = "app_name";
    roles[LastOpenedRole] = "last_opened";
    return roles;
}

/**
 * @brief Replaces the model contents with the provided items.
 * @param items Vector of recent log file records.
 */
auto RecentLogFilesModel::set_items(const QVector<RecentLogFileRecord>& items) -> void
{
    beginResetModel();
    m_items = items;
    endResetModel();
}

/**
 * @brief Clears all items.
 */
auto RecentLogFilesModel::clear() -> void
{
    beginResetModel();
    m_items.clear();
    endResetModel();
}

/**
 * @brief Appends a single item.
 * @param item The record to append.
 */
auto RecentLogFilesModel::add_item(const RecentLogFileRecord& item) -> void
{
    beginInsertRows(QModelIndex(), m_items.size(), m_items.size());
    m_items.append(item);
    endInsertRows();
}

/**
 * @brief Returns the item at the given row.
 * @param row Row index.
 * @return RecentLogFileRecord or a default-constructed one if out of range.
 */
auto RecentLogFilesModel::get_item(int row) const -> RecentLogFileRecord
{
    RecentLogFileRecord rec;

    if (row >= 0 && row < m_items.size())
    {
        rec = m_items.at(row);
    }

    return rec;
}

/**
 * @brief Extracts a file name from an absolute path.
 * @param file_path Absolute path.
 * @return File name part.
 */
auto RecentLogFilesModel::extract_file_name(const QString& file_path) -> QString
{
    const QString name = QFileInfo(file_path).fileName();
    return name;
}
