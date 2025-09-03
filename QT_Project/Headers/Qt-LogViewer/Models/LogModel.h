#pragma once

#include <QAbstractTableModel>
#include <QVector>

#include "Qt-LogViewer/Models/LogEntry.h"
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
};
