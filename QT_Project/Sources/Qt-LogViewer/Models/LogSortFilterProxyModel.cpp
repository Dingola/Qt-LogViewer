/**
 * @file LogSortFilterProxyModel.cpp
 * @brief This file contains the implementation of the LogSortFilterProxyModel class.
 */

#include "Qt-LogViewer/Models/LogSortFilterProxyModel.h"

#include "Qt-LogViewer/Models/LogModel.h"

/**
 * @brief Constructs a LogSortFilterProxyModel object.
 * @param parent The parent QObject.
 */
LogSortFilterProxyModel::LogSortFilterProxyModel(QObject* parent): QSortFilterProxyModel(parent)
{
    setSortRole(Qt::DisplayRole);
    setDynamicSortFilter(true);
}

/**
 * @brief Sets the application name filter.
 * @param app_name The application name to filter by (empty for no filter).
 */
auto LogSortFilterProxyModel::set_app_name_filter(const QString& app_name) -> void
{
    if (m_app_name_filter != app_name)
    {
        m_app_name_filter = app_name;
        invalidateFilter();
    }
}

/**
 * @brief Sets the set of log levels to filter by.
 * @param levels The set of log levels (e.g., {"Info", "Error"}).
 */
auto LogSortFilterProxyModel::set_level_filter(const QSet<QString>& levels) -> void
{
    if (m_level_filter != levels)
    {
        m_level_filter = levels;
        invalidateFilter();
    }
}

/**
 * @brief Sets the search string and field.
 * @param search_text The text or regex to search for.
 * @param field The field to search in ("Message", "Level", "AppName", etc.).
 * @param use_regex Whether to interpret search_text as a regular expression.
 */
auto LogSortFilterProxyModel::set_search_filter(const QString& search_text, const QString& field,
                                                bool use_regex) -> void
{
    bool changed =
        (m_search_text != search_text) || (m_search_field != field) || (m_use_regex != use_regex);

    if (changed)
    {
        m_search_text = search_text;
        m_search_field = field;
        m_use_regex = use_regex;

        if (m_use_regex && !m_search_text.isEmpty())
        {
            m_search_regex =
                QRegularExpression(m_search_text, QRegularExpression::CaseInsensitiveOption);
        }
        else
        {
            m_search_regex = QRegularExpression();
        }

        invalidateFilter();
    }
}

/**
 * @brief Determines whether the given row should be included in the filtered model.
 * @param source_row The row in the source model.
 * @param source_parent The parent index in the source model.
 * @return True if the row matches all filters, false otherwise.
 */
auto LogSortFilterProxyModel::filterAcceptsRow(int source_row,
                                               const QModelIndex& source_parent) const -> bool
{
    return row_passes_filter(source_row, source_parent);
}

/**
 * @brief Custom sorting logic for columns.
 * @param left The left index to compare.
 * @param right The right index to compare.
 * @return True if the left value is less than the right value.
 */
auto LogSortFilterProxyModel::lessThan(const QModelIndex& left,
                                       const QModelIndex& right) const -> bool
{
    QString lStr = sourceModel()->data(left, Qt::DisplayRole).toString();
    QString rStr = sourceModel()->data(right, Qt::DisplayRole).toString();

    return lStr.localeAwareCompare(rStr) < 0;
}

/**
 * @brief Checks if a specific row passes the current filters.
 */
auto LogSortFilterProxyModel::row_passes_filter(int row, const QModelIndex& parent) const -> bool
{
    bool accepted = true;

    QModelIndex index_app = sourceModel()->index(row, LogModel::AppName, parent);
    QModelIndex index_level = sourceModel()->index(row, LogModel::Level, parent);
    QModelIndex index_message = sourceModel()->index(row, LogModel::Message, parent);

    // App name filter
    if (!m_app_name_filter.isEmpty())
    {
        QString app_name = sourceModel()->data(index_app, Qt::DisplayRole).toString();
        if (app_name != m_app_name_filter)
        {
            accepted = false;
        }
    }

    // Level filter
    if (accepted && !m_level_filter.isEmpty())
    {
        QString level = sourceModel()->data(index_level, Qt::DisplayRole).toString();
        if (!m_level_filter.contains(level))
        {
            accepted = false;
        }
    }

    // Search filter
    if (accepted && !m_search_text.isEmpty())
    {
        QString value;
        if (m_search_field.compare("All Fields", Qt::CaseInsensitive) == 0)
        {
            value = sourceModel()->data(index_message, Qt::DisplayRole).toString() + " " +
                    sourceModel()->data(index_level, Qt::DisplayRole).toString() + " " +
                    sourceModel()->data(index_app, Qt::DisplayRole).toString();
        }
        else if (m_search_field.compare("Message", Qt::CaseInsensitive) == 0)
        {
            value = sourceModel()->data(index_message, Qt::DisplayRole).toString();
        }
        else if (m_search_field.compare("Level", Qt::CaseInsensitive) == 0)
        {
            value = sourceModel()->data(index_level, Qt::DisplayRole).toString();
        }
        else if (m_search_field.compare("AppName", Qt::CaseInsensitive) == 0)
        {
            value = sourceModel()->data(index_app, Qt::DisplayRole).toString();
        }
        else
        {
            value = sourceModel()->data(index_message, Qt::DisplayRole).toString() + " " +
                    sourceModel()->data(index_level, Qt::DisplayRole).toString() + " " +
                    sourceModel()->data(index_app, Qt::DisplayRole).toString();
        }

        if (m_use_regex)
        {
            if (!m_search_regex.isValid() || !m_search_regex.match(value).hasMatch())
            {
                accepted = false;
            }
        }
        else
        {
            if (!value.contains(m_search_text, Qt::CaseInsensitive))
            {
                accepted = false;
            }
        }
    }

    return accepted;
}
