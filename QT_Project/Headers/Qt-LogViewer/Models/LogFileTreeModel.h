#pragma once

#include <QAbstractItemModel>
#include <QHash>
#include <QMap>

class LogFileTreeItem;
class LogFileInfo;

/**
 * @class LogFileTreeModel
 * @brief Tree model for displaying sessions, application groups, and log files.
 *
 * This model organizes log files into a hierarchical structure:
 * - Root
 *   - Session(s)
 *     - Application Group(s)
 *       - Log File(s)
 *
 * Supports multiple sessions, each with its own set of application groups and files.
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
         * @brief Custom roles for data access.
         */
        enum Roles
        {
            ItemTypeRole = Qt::UserRole + 1,
            SessionIdRole,
            FilePathRole,
            AppNameRole
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

        // -------------------------------------------------------------------------
        // Session management
        // -------------------------------------------------------------------------

        /**
         * @brief Adds a new session to the tree.
         * @param session_id Unique identifier for the session.
         * @param session_name Display name for the session.
         * @return True if the session was added; false if it already exists.
         */
        auto add_session(const QString& session_id, const QString& session_name) -> bool;

        /**
         * @brief Removes a session and all its children from the tree.
         * @param session_id The session identifier.
         * @return True if the session was removed; false if not found.
         */
        auto remove_session(const QString& session_id) -> bool;

        /**
         * @brief Renames an existing session.
         * @param session_id The session identifier.
         * @param new_name The new display name.
         * @return True if renamed; false if session not found.
         */
        auto rename_session(const QString& session_id, const QString& new_name) -> bool;

        /**
         * @brief Checks if a session exists.
         * @param session_id The session identifier.
         * @return True if the session exists.
         */
        [[nodiscard]] auto has_session(const QString& session_id) const -> bool;

        /**
         * @brief Returns the number of sessions in the model.
         * @return The session count.
         */
        [[nodiscard]] auto get_session_count() const -> int;

        /**
         * @brief Returns the model index for a session.
         * @param session_id The session identifier.
         * @return The model index, or invalid if not found.
         */
        [[nodiscard]] auto get_session_index(const QString& session_id) const -> QModelIndex;

        // -------------------------------------------------------------------------
        // Log file management (session-aware)
        // -------------------------------------------------------------------------

        /**
         * @brief Sets the log files for a specific session (replaces existing files).
         * @param session_id The session identifier.
         * @param log_file_infos The list of LogFileInfo objects.
         */
        auto set_log_files(const QString& session_id,
                           const QList<LogFileInfo>& log_file_infos) -> void;

        /**
         * @brief Adds a single log file to a session.
         * @param session_id The session identifier.
         * @param log_file_info The LogFileInfo to add.
         * @return True if added; false if session not found or file already exists.
         */
        auto add_log_file(const QString& session_id, const LogFileInfo& log_file_info) -> bool;

        /**
         * @brief Adds multiple log files to a session.
         * @param session_id The session identifier.
         * @param log_file_infos The list of LogFileInfo objects to add.
         * @return Number of files successfully added.
         */
        auto add_log_files(const QString& session_id,
                           const QList<LogFileInfo>& log_file_infos) -> int;

        /**
         * @brief Removes a single log file from a session.
         * @param session_id The session identifier.
         * @param log_file_info The LogFileInfo to remove.
         * @return True if removed; false if not found.
         */
        auto remove_log_file(const QString& session_id, const LogFileInfo& log_file_info) -> bool;

        /**
         * @brief Clears all log files from a session (keeps the session node).
         * @param session_id The session identifier.
         */
        auto clear_session_files(const QString& session_id) -> void;

        // -------------------------------------------------------------------------
        // Log file management (all sessions)
        // -------------------------------------------------------------------------

        /**
         * @brief Sets the log files for all existing sessions (replaces existing files).
         * @param log_file_infos The list of LogFileInfo objects.
         */
        auto set_log_files(const QList<LogFileInfo>& log_file_infos) -> void;

        /**
         * @brief Adds a single log file to all existing sessions.
         * @param log_file_info The LogFileInfo to add.
         */
        auto add_log_file(const LogFileInfo& log_file_info) -> void;

        /**
         * @brief Removes a single log file from all sessions where it exists.
         * @param log_file_info The LogFileInfo to remove.
         */
        auto remove_log_file(const LogFileInfo& log_file_info) -> void;

        // -------------------------------------------------------------------------
        // QAbstractItemModel overrides
        // -------------------------------------------------------------------------

        /**
         * @brief Returns the number of columns for the given parent.
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
         * @brief Sets the data for the given index and role.
         * @param index The model index.
         * @param value The value to set.
         * @param role The data role.
         * @return True if the data was set successfully; false otherwise.
         */
        auto setData(const QModelIndex& index, const QVariant& value,
                     int role = Qt::EditRole) -> bool override;

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

    signals:
        /**
         * @brief Emitted when the last session is removed from the model.
         */
        void all_sessions_removed();

    private:
        /**
         * @brief Key for identifying a group within a session.
         */
        struct GroupKey {
                QString session_id;
                QString app_name;

                auto operator==(const GroupKey& other) const -> bool
                {
                    return session_id == other.session_id && app_name == other.app_name;
                }
        };

        /**
         * @brief Hash function for GroupKey.
         */
        friend auto qHash(const GroupKey& key, size_t seed = 0) -> size_t
        {
            return qHash(key.session_id, seed) ^ qHash(key.app_name, seed);
        }

        /**
         * @brief Helper to group files by application name.
         * @param files The list of LogFileInfo objects.
         * @return A map from app name to list of files.
         */
        [[nodiscard]] static auto group_by_app_name(const QList<LogFileInfo>& files)
            -> QMap<QString, QList<LogFileInfo>>;

        /**
         * @brief Finds the row of a file in a group by file path.
         * @param group_item The group item.
         * @param file_path The file path to search for.
         * @return The row index, or -1 if not found.
         */
        [[nodiscard]] auto find_file_row(LogFileTreeItem* group_item,
                                         const QString& file_path) const -> int;

        /**
         * @brief Finds or creates an application group under a session.
         * @param session_item The session item.
         * @param session_id The session identifier (for caching).
         * @param app_name The application name.
         * @return Pointer to the group item.
         */
        auto find_or_create_group(LogFileTreeItem* session_item, const QString& session_id,
                                  const QString& app_name) -> LogFileTreeItem*;

        /**
         * @brief Removes empty groups from a session.
         * @param session_item The session item to clean up.
         * @param session_id The session identifier.
         */
        auto remove_empty_groups(LogFileTreeItem* session_item, const QString& session_id) -> void;

        /**
         * @brief Gets the row index of a session item.
         * @param session_item The session item.
         * @return The row index.
         */
        [[nodiscard]] auto get_session_row(LogFileTreeItem* session_item) const -> int;

    private:
        LogFileTreeItem* m_root_item;
        QHash<QString, LogFileTreeItem*> m_session_items;
        QHash<GroupKey, LogFileTreeItem*> m_group_items;
};
