/**
 * @file RecentSessionsModel.cpp
 * @brief Implements the RecentSessionsModel used to display recent sessions.
 */

#include "Qt-LogViewer/Models/RecentSessionsModel.h"

/**
 * @brief Constructs an empty RecentSessionsModel.
 * @param parent Optional QObject parent.
 */
RecentSessionsModel::RecentSessionsModel(QObject* parent): QAbstractTableModel(parent), m_items() {}

/**
 * @brief Returns the number of rows.
 * @param parent The parent index (unused for a flat model).
 * @return Number of items in the model.
 */
auto RecentSessionsModel::rowCount(const QModelIndex& parent) const -> int
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
auto RecentSessionsModel::columnCount(const QModelIndex& parent) const -> int
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
auto RecentSessionsModel::data(const QModelIndex& index, int role) const -> QVariant
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size())
    {
        return {};
    }

    const RecentSessionRecord& rec = m_items.at(index.row());

    if (role == Qt::DisplayRole || role == Qt::EditRole)
    {
        switch (index.column())
        {
        case Name:
            return rec.name;
        case LastOpened:
            return rec.last_opened;
        case Id:
            return rec.id;
        default:
            return {};
        }
    }

    switch (role)
    {
    case NameRole:
        return rec.name;
    case LastOpenedRole:
        return rec.last_opened;
    case IdRole:
        return rec.id;
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
auto RecentSessionsModel::headerData(int section, Qt::Orientation orientation,
                                     int role) const -> QVariant
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
    {
        return {};
    }

    switch (section)
    {
    case Name:
        return QStringLiteral("Session Name");
    case LastOpened:
        return QStringLiteral("Last Opened");
    case Id:
        return QStringLiteral("Session Id");
    default:
        return {};
    }
}

/**
 * @brief Returns item flags.
 * @param index The model index.
 * @return Selectable + enabled for valid items.
 */
auto RecentSessionsModel::flags(const QModelIndex& index) const -> Qt::ItemFlags
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
auto RecentSessionsModel::roleNames() const -> QHash<int, QByteArray>
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[LastOpenedRole] = "last_opened";
    roles[IdRole] = "id";
    return roles;
}

/**
 * @brief Replaces the model contents with the provided items.
 * @param items Vector of recent session records.
 */
auto RecentSessionsModel::set_items(const QVector<RecentSessionRecord>& items) -> void
{
    beginResetModel();
    m_items = items;
    endResetModel();
}

/**
 * @brief Clears all items.
 */
auto RecentSessionsModel::clear() -> void
{
    beginResetModel();
    m_items.clear();
    endResetModel();
}

/**
 * @brief Appends a single item.
 * @param item The record to append.
 */
auto RecentSessionsModel::add_item(const RecentSessionRecord& item) -> void
{
    beginInsertRows(QModelIndex(), m_items.size(), m_items.size());
    m_items.append(item);
    endInsertRows();
}

/**
 * @brief Returns the item at the given row.
 * @param row Row index.
 * @return RecentSessionRecord or a default-constructed one if out of range.
 */
auto RecentSessionsModel::get_item(int row) const -> RecentSessionRecord
{
    RecentSessionRecord rec;

    if (row >= 0 && row < m_items.size())
    {
        rec = m_items.at(row);
    }

    return rec;
}
