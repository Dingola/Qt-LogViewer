#pragma once

#include <QRegularExpression>
#include <QSet>
#include <QSortFilterProxyModel>
#include <QString>

/**
 * @file LogSortFilterProxyModel.h
 * @brief This file contains the definition of the LogSortFilterProxyModel class.
 */

/**
 * @class LogSortFilterProxyModel
 * @brief Proxy model for filtering and sorting log entries in the LogModel.
 *
 * Supports filtering by application name, log level, search string (plain or regex),
 * and custom sorting (e.g., log levels, timestamps).
 */
class LogSortFilterProxyModel: public QSortFilterProxyModel
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs a LogSortFilterProxyModel object.
         * @param parent The parent QObject.
         */
        explicit LogSortFilterProxyModel(QObject* parent = nullptr);

        /**
         * @brief Sets the application name filter.
         * @param app_name The application name to filter by (empty for no filter).
         */
        auto set_app_name_filter(const QString& app_name) -> void;

        /**
         * @brief Sets the set of log levels to filter by.
         * @param levels The set of log levels (e.g., {"Info", "Error"}).
         */
        auto set_level_filter(const QSet<QString>& levels) -> void;

        /**
         * @brief Sets the search string and field.
         * @param search_text The text or regex to search for.
         * @param field The field to search in ("Message", "Level", "AppName", etc.).
         * @param use_regex Whether to interpret search_text as a regular expression.
         */
        auto set_search_filter(const QString& search_text, const QString& field,
                               bool use_regex) -> void;

    protected:
        /**
         * @brief Determines whether the given row should be included in the filtered model.
         * @param source_row The row in the source model.
         * @param source_parent The parent index in the source model.
         * @return True if the row matches all filters, false otherwise.
         */
        auto filterAcceptsRow(int source_row,
                              const QModelIndex& source_parent) const -> bool override;

        /**
         * @brief Custom sorting logic for columns.
         * @param left The left index to compare.
         * @param right The right index to compare.
         * @return True if the left value is less than the right value.
         */
        auto lessThan(const QModelIndex& left, const QModelIndex& right) const -> bool override;

    private:
        /**
         * @brief Checks if a specific row passes the current filters.
         * @param row The row in the source model.
         * @param parent The parent index in the source model.
         */
        [[nodiscard]] auto row_passes_filter(int row, const QModelIndex& parent) const -> bool;

    private:
        QString m_app_name_filter;
        QSet<QString> m_level_filter;
        QString m_search_text;
        QString m_search_field;
        bool m_use_regex = false;
        QRegularExpression m_search_regex;
};
