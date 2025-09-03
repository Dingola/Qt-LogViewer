#pragma once

#include <QVariant>
#include <QVector>

/**
 * @class LogFileTreeItem
 * @brief Represents a single item in the log file tree model.
 *
 * Each item can have multiple columns and child items.
 */
class LogFileTreeItem
{
    public:
        /**
         * @brief The type of the tree item (group or file).
         */
        enum class Type
        {
            Group,  ///< Represents an application group node.
            File    ///< Represents a log file node.
        };

        /**
         * @brief Constructs a LogFileTreeItem.
         * @param data The list of column data for this item.
         * @param parent The parent item, or nullptr.
         */
        explicit LogFileTreeItem(const QVector<QVariant>& data, LogFileTreeItem* parent = nullptr);

        /**
         * @brief Destroys the LogFileTreeItem and its children.
         */
        ~LogFileTreeItem();

        /**
         * @brief Appends a child item.
         * @param child The child item to append.
         */
        auto append_child(LogFileTreeItem* child) -> void;

        /**
         * @brief Removes the child at the given row.
         * @param row The child row index.
         * @return Pointer to the removed child item, or nullptr if out of bounds.
         */
        auto remove_child(int row) -> LogFileTreeItem*;

        /**
         * @brief Returns the child at the given row.
         * @param row The child row index.
         * @return Pointer to the child item, or nullptr.
         */
        [[nodiscard]] auto child(int row) const -> LogFileTreeItem*;

        /**
         * @brief Returns the number of child items.
         * @return The number of children.
         */
        [[nodiscard]] auto child_count() const -> int;

        /**
         * @brief Returns the number of columns.
         * @return The number of columns.
         */
        [[nodiscard]] auto column_count() const -> int;

        /**
         * @brief Returns the data for the given column.
         * @param column The column index.
         * @return The data for the column.
         */
        [[nodiscard]] auto data(int column) const -> QVariant;

        /**
         * @brief Returns the parent item.
         * @return Pointer to the parent item, or nullptr.
         */
        [[nodiscard]] auto parent_item() const -> LogFileTreeItem*;

        /**
         * @brief Returns the row index of this item in its parent.
         * @return The row index.
         */
        [[nodiscard]] auto row() const -> int;

    private:
        QVector<LogFileTreeItem*> m_child_items;
        QVector<QVariant> m_item_data;
        LogFileTreeItem* m_parent_item;
};

Q_DECLARE_METATYPE(LogFileTreeItem::Type)
