/**
 * @file RecentItemsModel.cpp
 * @brief Implements the generic, schema-driven recent items model.
 *
 * The model is domain-agnostic. A RecentListSchema provides exported role names and column
 * definitions. Callers supply rows via set_rows(...), where each row is a role->QVariant map.
 */

#include "Qt-LogViewer/Models/RecentItemsModel.h"

#include "Qt-LogViewer/Models/RecentListSchema.h"

/**
 * @brief Constructs a recent items model with the given schema.
 * @param schema Schema defining exported roles and columns.
 * @param parent Optional QObject parent.
 *
 * The schema determines which roles are exported via roleNames() and how columns are presented.
 */
RecentItemsModel::RecentItemsModel(const RecentListSchema& schema, QObject* parent)
    : QAbstractTableModel(parent), m_schema(schema), m_rows()
{}

/**
 * @brief Returns the number of rows.
 * @param parent Parent index (unused for flat model).
 * @return Number of rows.
 */
auto RecentItemsModel::rowCount(const QModelIndex& parent) const -> int
{
    int count = 0;

    if (!parent.isValid())
    {
        count = m_rows.size();
    }

    return count;
}

/**
 * @brief Returns the number of columns defined by the schema.
 * @param parent Parent index (unused).
 * @return Column count.
 */
auto RecentItemsModel::columnCount(const QModelIndex& parent) const -> int
{
    Q_UNUSED(parent);
    int cols = 0;
    cols = m_schema.columns.size();
    return cols;
}

/**
 * @brief Returns data for the given index and role.
 * @param index Model index (row/column).
 * @param role Qt role or custom role.
 * @return Requested value or invalid QVariant if out of range.
 *
 * For Qt::DisplayRole and Qt::EditRole, the value for the column's display role is returned.
 * For other roles, the row's role map is queried directly.
 */
auto RecentItemsModel::data(const QModelIndex& index, int role) const -> QVariant
{
    QVariant value;

    if (index.isValid() && index.row() >= 0 && index.row() < m_rows.size())
    {
        if (index.column() >= 0 && index.column() < m_schema.columns.size())
        {
            const auto& row = m_rows.at(index.row());

            if (role == Qt::DisplayRole || role == Qt::EditRole)
            {
                const int display_role = m_schema.columns.at(index.column()).display_role;
                value = row.value(display_role);
            }
            else
            {
                value = row.value(role);
            }
        }
    }

    return value;
}

/**
 * @brief Returns header text for columns.
 * @param section Column index.
 * @param orientation Expected Qt::Horizontal.
 * @param role Expected Qt::DisplayRole.
 * @return Header text or invalid QVariant if out of range.
 */
auto RecentItemsModel::headerData(int section, Qt::Orientation orientation,
                                  int role) const -> QVariant
{
    QVariant header;

    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        if (section >= 0 && section < m_schema.columns.size())
        {
            header = m_schema.columns.at(section).header_title;
        }
    }

    return header;
}

/**
 * @brief Returns item flags for a given index.
 * @param index Model index.
 * @return Selectable and enabled flags for valid indices; Qt::NoItemFlags otherwise.
 */
auto RecentItemsModel::flags(const QModelIndex& index) const -> Qt::ItemFlags
{
    Qt::ItemFlags item_flags = Qt::NoItemFlags;

    if (index.isValid())
    {
        item_flags = (Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    }

    return item_flags;
}

/**
 * @brief Exposes the role names defined by the active schema.
 * @return Mapping of role id to role name.
 */
auto RecentItemsModel::roleNames() const -> QHash<int, QByteArray>
{
    return m_schema.role_names;
}

/**
 * @brief Replaces the model contents with the provided rows.
 * @param rows New rows to set (role->QVariant maps).
 *
 * This call resets the model atomically. Rows should be consistent with the schema's roles.
 */
auto RecentItemsModel::set_rows(QVector<QHash<int, QVariant>> rows) -> void
{
    beginResetModel();
    m_rows = std::move(rows);
    endResetModel();
}

/**
 * @brief Clears all rows in the model.
 *
 * Resets the model and removes all row data.
 */
auto RecentItemsModel::clear() -> void
{
    beginResetModel();
    m_rows.clear();
    endResetModel();
}

/**
 * @brief Returns a copy of the active schema used by the model.
 * @return Schema copy (roles and columns).
 */
auto RecentItemsModel::get_schema() const -> RecentListSchema
{
    RecentListSchema schema_copy = m_schema;
    return schema_copy;
}
