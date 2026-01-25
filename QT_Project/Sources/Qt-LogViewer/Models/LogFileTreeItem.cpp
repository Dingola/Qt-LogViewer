#include "Qt-LogViewer/Models/LogFileTreeItem.h"

/**
 * @brief Constructs a LogFileTreeItem.
 * @param data The list of column data for this item.
 * @param parent The parent item, or nullptr.
 */
LogFileTreeItem::LogFileTreeItem(const QVector<QVariant>& data, LogFileTreeItem* parent)
    : m_item_data(data), m_parent_item(parent)
{}

/**
 * @brief Destroys the LogFileTreeItem and its children.
 */
LogFileTreeItem::~LogFileTreeItem()
{
    for (auto* child: m_child_items)
    {
        delete child;
    }
}

/**
 * @brief Appends a child item.
 * @param child The child item to append.
 */
auto LogFileTreeItem::append_child(LogFileTreeItem* child) -> void
{
    m_child_items.append(child);
}

/**
 * @brief Removes the child at the given row.
 * @param row The child row index.
 * @return Pointer to the removed child item, or nullptr if out of bounds.
 */
auto LogFileTreeItem::remove_child(int row) -> LogFileTreeItem*
{
    LogFileTreeItem* removed = nullptr;

    if (row >= 0 && row < m_child_items.size())
    {
        removed = m_child_items.at(row);
        m_child_items.removeAt(row);
    }

    return removed;
}

/**
 * @brief Returns the child at the given row.
 * @param row The child row index.
 * @return Pointer to the child item, or nullptr.
 */
auto LogFileTreeItem::child(int row) const -> LogFileTreeItem*
{
    LogFileTreeItem* result = nullptr;

    if (row >= 0 && row < m_child_items.size())
    {
        result = m_child_items.at(row);
    }

    return result;
}

/**
 * @brief Returns the number of child items.
 * @return The number of children.
 */
auto LogFileTreeItem::child_count() const -> int
{
    return m_child_items.size();
}

/**
 * @brief Returns the number of columns.
 * @return The number of columns.
 */
auto LogFileTreeItem::column_count() const -> int
{
    return m_item_data.size();
}

/**
 * @brief Returns the data for the given column.
 * @param column The column index.
 * @return The data for the column.
 */
auto LogFileTreeItem::data(int column) const -> QVariant
{
    QVariant result;

    if (column >= 0 && column < m_item_data.size())
    {
        result = m_item_data.at(column);
    }

    return result;
}

/**
 * @brief Sets the data for the given column.
 * @param column The column index.
 * @param value The value to set.
 * @return True if the data was set successfully; false otherwise.
 */
auto LogFileTreeItem::set_data(int column, const QVariant& value) -> bool
{
    bool success = false;

    if (column >= 0 && column < m_item_data.size())
    {
        m_item_data[column] = value;
        success = true;
    }

    return success;
}

/**
 * @brief Returns the parent item.
 * @return Pointer to the parent item, or nullptr.
 */
auto LogFileTreeItem::parent_item() const -> LogFileTreeItem*
{
    return m_parent_item;
}

/**
 * @brief Returns the row index of this item in its parent.
 * @return The row index.
 */
auto LogFileTreeItem::row() const -> int
{
    int result = 0;

    if (m_parent_item)
    {
        result = m_parent_item->child_count() > 0
                     ? m_parent_item->m_child_items.indexOf(const_cast<LogFileTreeItem*>(this))
                     : 0;
    }

    return result;
}
