#pragma once

#include <QRegularExpression>
#include <QSet>
#include <QSortFilterProxyModel>
#include <QString>

/**
 * @file LogFilterProxyModel.h
 * @brief This file contains the definition of the LogFilterProxyModel class.
 */

/**
 * @class LogFilterProxyModel
 * @brief Proxy model for filtering and searching log entries in the LogModel.
 *
 * Supports filtering by application name, log level, and search string (plain or regex)
 * in a selected field.
 */
class LogFilterProxyModel: public QSortFilterProxyModel
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs a LogFilterProxyModel object.
         * @param parent The parent QObject.
         */
        explicit LogFilterProxyModel(QObject* parent = nullptr);

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

    private:
        QString m_app_name_filter;
        QSet<QString> m_level_filter;
        QString m_search_text;
        QString m_search_field;
        bool m_use_regex = false;
        QRegularExpression m_search_regex;
};
