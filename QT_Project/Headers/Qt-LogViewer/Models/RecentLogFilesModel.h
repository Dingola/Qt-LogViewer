#pragma once

#include <QAbstractTableModel>
#include <QDateTime>
#include <QVector>

#include "Qt-LogViewer/Models/SessionTypes.h"

/**
 * @file RecentLogFilesModel.h
 * @brief Declares the RecentLogFilesModel for displaying recent log files (path, app, last opened).
 *
 * This model is intended for binding to menus or a start page. It exposes both column-based
 * data for views and custom roles for programmatic access.
 *
 * Columns:
 * - FilePath
 * - FileName
 * - AppName
 * - LastOpened
 *
 * Roles:
 * - FilePathRole
 * - FileNameRole
 * - AppNameRole
 * - LastOpenedRole
 */
class RecentLogFilesModel: public QAbstractTableModel
{
        Q_OBJECT

    public:
        /**
         * @brief Columns for the table model.
         */
        enum Column
        {
            FilePath = 0,
            FileName,
            AppName,
            LastOpened,
            ColumnCount
        };

        /**
         * @brief Custom roles for direct access to values.
         */
        enum Role
        {
            FilePathRole = Qt::UserRole + 1,
            FileNameRole,
            AppNameRole,
            LastOpenedRole
        };

    public:
        /**
         * @brief Constructs an empty RecentLogFilesModel.
         * @param parent Optional QObject parent.
         */
        explicit RecentLogFilesModel(QObject* parent = nullptr);

        /**
         * @brief Default destructor.
         */
        ~RecentLogFilesModel() override = default;

        /**
         * @brief Returns the number of rows.
         * @param parent The parent index (unused for a flat model).
         * @return Number of items in the model.
         */
        [[nodiscard]] auto rowCount(const QModelIndex& parent = QModelIndex()) const
            -> int override;

        /**
         * @brief Returns the number of columns.
         * @param parent The parent index (unused).
         * @return Number of columns.
         */
        [[nodiscard]] auto columnCount(const QModelIndex& parent = QModelIndex()) const
            -> int override;

        /**
         * @brief Returns data for the given index/role.
         * @param index Model index.
         * @param role Qt role or custom role.
         * @return Data as QVariant; invalid if out of range or role not supported.
         */
        [[nodiscard]] auto data(const QModelIndex& index,
                                int role = Qt::DisplayRole) const -> QVariant override;

        /**
         * @brief Returns header data for columns.
         * @param section Column index.
         * @param orientation Qt::Horizontal expected.
         * @param role Qt role.
         * @return Header text or invalid QVariant.
         */
        [[nodiscard]] auto headerData(int section, Qt::Orientation orientation,
                                      int role = Qt::DisplayRole) const -> QVariant override;

        /**
         * @brief Returns item flags.
         * @param index The model index.
         * @return Selectable + enabled for valid items.
         */
        [[nodiscard]] auto flags(const QModelIndex& index) const -> Qt::ItemFlags override;

        /**
         * @brief Returns role names for custom roles.
         * @return Mapping of role ids to names.
         */
        [[nodiscard]] auto roleNames() const -> QHash<int, QByteArray> override;

        /**
         * @brief Replaces the model contents with the provided items.
         * @param items Vector of recent log file records.
         */
        auto set_items(const QVector<RecentLogFileRecord>& items) -> void;

        /**
         * @brief Clears all items.
         */
        auto clear() -> void;

        /**
         * @brief Appends a single item.
         * @param item The record to append.
         */
        auto add_item(const RecentLogFileRecord& item) -> void;

        /**
         * @brief Returns the item at the given row.
         * @param row Row index.
         * @return RecentLogFileRecord or a default-constructed one if out of range.
         */
        [[nodiscard]] auto get_item(int row) const -> RecentLogFileRecord;

    private:
        /**
         * @brief Extracts a file name from an absolute path.
         * @param file_path Absolute path.
         * @return File name part.
         */
        [[nodiscard]] static auto extract_file_name(const QString& file_path) -> QString;

    private:
        QVector<RecentLogFileRecord> m_items;
};
