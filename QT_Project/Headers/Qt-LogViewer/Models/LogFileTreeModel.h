#pragma once

#include <QAbstractItemModel>

class LogFileTreeItem;
class LogFileInfo;

/**
 * @class LogFileTreeModel
 * @brief Tree model for grouping log files by application name, supporting multiple columns.
 *
 * This model organizes log files into groups by application name. Each file entry can have
 * multiple columns (currently only file name).
 */
class LogFileTreeModel: public QAbstractItemModel
{
        Q_OBJECT

    public:
        /**
         * @brief Column enum for easier access.
         */
        enum Column
        {
            Name = 0,
            ColumnCount
        };

        /**
         * @brief Constructs a LogFileTreeModel object.
         * @param parent The parent QObject, or nullptr.
         */
        explicit LogFileTreeModel(QObject* parent = nullptr);

        /**
         * @brief Destroys the LogFileTreeModel object.
         */
        ~LogFileTreeModel() override;

        /**
         * @brief Sets the log files to be displayed and grouped.
         * @param files The list of LogFileInfo objects.
         */
        auto set_log_files(const QList<LogFileInfo>& files) -> void;

        /**
         * @brief Adds a single log file to the model. Creates a new group if necessary.
         * @param file The LogFileInfo to add.
         */
        auto add_log_file(const LogFileInfo& file) -> void;

        /**
         * @brief Removes a single log file from the model. Removes the group if it becomes empty.
         * @param file The LogFileInfo to remove.
         */
        auto remove_log_file(const LogFileInfo& file) -> void;

        /**
         * @brief Returns the number of columns.
         * @param parent The parent index.
         * @return The number of columns.
         */
        [[nodiscard]] auto columnCount(const QModelIndex& parent = QModelIndex()) const
            -> int override;

        /**
         * @brief Returns the number of rows under the given parent.
         * @param parent The parent index.
         * @return The number of child rows.
         */
        [[nodiscard]] auto rowCount(const QModelIndex& parent = QModelIndex()) const
            -> int override;

        /**
         * @brief Returns the data for the given index and role.
         * @param index The model index.
         * @param role The data role.
         * @return The data value for the given index and role.
         */
        [[nodiscard]] auto data(const QModelIndex& index,
                                int role = Qt::DisplayRole) const -> QVariant override;

        /**
         * @brief Returns the item flags for the given index.
         * @param index The model index.
         * @return The item flags.
         */
        [[nodiscard]] auto flags(const QModelIndex& index) const -> Qt::ItemFlags override;

        /**
         * @brief Returns the header data for the given section, orientation, and role.
         * @param section The section index.
         * @param orientation The orientation.
         * @param role The data role.
         * @return The header text for the given section.
         */
        [[nodiscard]] auto headerData(int section, Qt::Orientation orientation,
                                      int role = Qt::DisplayRole) const -> QVariant override;

        /**
         * @brief Returns the index for the given row, column, and parent.
         * @param row The row index.
         * @param column The column index.
         * @param parent The parent index.
         * @return The model index.
         */
        [[nodiscard]] auto index(int row, int column, const QModelIndex& parent = QModelIndex())
            const -> QModelIndex override;

        /**
         * @brief Returns the parent index for the given index.
         * @param index The model index.
         * @return The parent index.
         */
        [[nodiscard]] auto parent(const QModelIndex& index) const -> QModelIndex override;

    private:
        /**
         * @brief Helper to group files by application name.
         * @param files The list of LogFileInfo objects.
         * @return A map from app name to list of files.
         */
        [[nodiscard]] static auto group_by_app_name(const QList<LogFileInfo>& files)
            -> QMap<QString, QList<LogFileInfo>>;

        /**
         * @brief Finds the row of a file in a group by file name.
         * @param group_item The group item.
         * @param file_name The file name to search for.
         * @return The row index, or -1 if not found.
         */
        [[nodiscard]] auto find_file_row(LogFileTreeItem* group_item,
                                         const QString& file_name) const -> int;

    private:
        LogFileTreeItem* m_root_item;
        QHash<QString, LogFileTreeItem*> m_group_items;  // app name -> group item
};
