#pragma once

#include <QAbstractTableModel>
#include <QHash>
#include <QVariant>
#include <QVector>

#include "Qt-LogViewer/Models/RecentListSchema.h"

/**
 * @file RecentItemsModel.h
 * @brief Generic, schema-driven model for recent items (files, sessions, future lists).
 *
 * The model is configured via a RecentListSchema. Each row is represented as a role->QVariant
 * map. This model does not hardcode any list type; schemas define roles and columns. Callers supply
 * rows built from their own domain objects (for example, RecentLogFileRecord, RecentSessionRecord)
 * via the generic set_rows(...) API.
 */
class RecentItemsModel: public QAbstractTableModel
{
        Q_OBJECT

    public:
        /**
         * @brief Construct a model with a given schema.
         * @param schema Schema defining exported roles and columns.
         * @param parent Optional QObject parent.
         */
        explicit RecentItemsModel(const RecentListSchema& schema, QObject* parent = nullptr);

        /**
         * @brief Returns the number of rows.
         * @param parent Parent index (unused for flat model).
         */
        auto rowCount(const QModelIndex& parent = QModelIndex()) const -> int override;

        /**
         * @brief Returns the number of columns defined by the schema.
         * @param parent Parent index (unused).
         */
        auto columnCount(const QModelIndex& parent = QModelIndex()) const -> int override;

        /**
         * @brief Returns data for the given index/role.
         * @param index Model index.
         * @param role Qt role or custom role.
         */
        auto data(const QModelIndex& index, int role = Qt::DisplayRole) const -> QVariant override;

        /**
         * @brief Returns header data for columns (Qt::Horizontal + DisplayRole).
         * @param section Column index.
         * @param orientation Qt::Horizontal expected.
         * @param role Qt::DisplayRole expected.
         */
        auto headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const -> QVariant override;

        /**
         * @brief Item flags: selectable and enabled for valid rows.
         * @param index Model index.
         */
        auto flags(const QModelIndex& index) const -> Qt::ItemFlags override;

        /**
         * @brief Exposes role names from the configured schema.
         * @return Role id to role name mapping.
         */
        auto roleNames() const -> QHash<int, QByteArray> override;

        /**
         * @brief Replace contents with new rows.
         *
         * Rows are provided as QHash<int,QVariant> maps where keys are role ids and values are
         * role payloads. The rows should be consistent with the schema's exported roles.
         *
         * @param rows New rows to set (role->QVariant maps).
         */
        auto set_rows(QVector<QHash<int, QVariant>> rows) -> void;

        /**
         * @brief Clears all rows.
         */
        auto clear() -> void;

        /**
         * @brief Returns a copy of the active schema.
         */
        [[nodiscard]] auto get_schema() const -> RecentListSchema;

    private:
        RecentListSchema m_schema;             ///< Active schema (roles and columns).
        QVector<QHash<int, QVariant>> m_rows;  ///< Row storage: role->QVariant maps.
};
