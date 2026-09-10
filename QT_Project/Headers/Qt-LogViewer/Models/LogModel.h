#pragma once

#include <QAbstractTableModel>
#include <QVector>

#include "Qt-LogViewer/Models/LogEntry.h"
#include "Qt-LogViewer/Models/LogFieldDefinition.h"
#include "SimpleCppLogger/LogLevel.h"

// LogModel: Model for displaying and managing log entries in a QTableView.
class LogModel: public QAbstractTableModel
{
        Q_OBJECT

    public:
        // Column and role enums
        enum Column
        {
            Timestamp = 0,
            Level,
            Message,
            AppName,
            Spacer,
            ColumnCount
        };

        enum LogRole
        {
            TimestampRole = Qt::UserRole + 1,
            LevelRole,
            MessageRole,
            AppNameRole
        };

        // Required overrides for QAbstractTableModel
        explicit LogModel(QObject* parent = nullptr);
        ~LogModel() override = default;

        [[nodiscard]] auto rowCount(const QModelIndex& parent = QModelIndex()) const
            -> int override;
        [[nodiscard]] auto columnCount(const QModelIndex& parent = QModelIndex()) const
            -> int override;
        [[nodiscard]] auto data(const QModelIndex& index,
                                int role = Qt::DisplayRole) const -> QVariant override;
        [[nodiscard]] auto headerData(int section, Qt::Orientation orientation,
                                      int role = Qt::DisplayRole) const -> QVariant override;
        [[nodiscard]] auto flags(const QModelIndex& index) const -> Qt::ItemFlags override;
        [[nodiscard]] auto roleNames() const -> QHash<int, QByteArray> override;

        /**
         * @brief Replaces the columns displayed by this model.
         * @param
         * columns Ordered field definitions used for table data and headers.
         */
        auto set_columns(const QVector<LogFieldDefinition>& columns) -> void;

        /**
         * @brief Appends columns whose field identifiers are not already present.

         * * @param columns Ordered field definitions to merge into the current schema.
         */
        auto append_columns(const QVector<LogFieldDefinition>& columns) -> void;

        /**
         * @brief Returns the stable field identifier represented by a model column.

         * * @param column Zero-based model column.
         * @return Field identifier, or an empty
         * string for an invalid or spacer column.
         */
        [[nodiscard]] auto get_column_field_id(int column) const -> QString;

        /**
         * @brief Finds the model column representing a stable field identifier.

         * * @param field_id Stable parser or built-in field identifier.
         * @return
         * Zero-based column, or -1 when the field is not displayed.
         */
        [[nodiscard]] auto find_column(const QString& field_id) const -> int;

        /**
         * @brief Returns whether a displayed column supports database sorting.

         * * @param column Zero-based model column.
         * @return True when the column has a
         * sortable field definition.
         */
        [[nodiscard]] auto is_column_sortable(int column) const -> bool;

        // Custom methods to interact with log entries
        auto add_entry(const LogEntry& entry) -> void;
        auto clear() -> void;
        [[nodiscard]] auto get_entry(int row) const -> LogEntry;
        [[nodiscard]] auto get_entries() const -> QVector<LogEntry>;
        auto add_entries(const QVector<LogEntry>& entries) -> void;
        auto set_entries(const QVector<LogEntry>& entries) -> void;

        /**
         * @brief Removes all log entries associated with the given file path.
         * @param file_path The file path whose log entries should be removed.
         */
        auto remove_entries_by_file_path(const QString& file_path) -> void;

    private:
        static auto map_log_level(const QString& level_str) -> SimpleCppLogger::LogLevel;

    private:
        QVector<LogEntry> m_entries;
        QVector<LogFieldDefinition> m_columns;
};
