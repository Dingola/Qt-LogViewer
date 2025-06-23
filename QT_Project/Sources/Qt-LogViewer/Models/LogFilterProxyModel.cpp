/**
 * @file LogFilterProxyModel.cpp
 * @brief This file contains the implementation of the LogFilterProxyModel class.
 */

#include "Qt-LogViewer/Models/LogFilterProxyModel.h"

#include "Qt-LogViewer/Models/LogModel.h"

/**
 * @brief Constructs a LogFilterProxyModel object.
 * @param parent The parent QObject.
 */
LogFilterProxyModel::LogFilterProxyModel(QObject* parent): QSortFilterProxyModel(parent) {}

/**
 * @brief Sets the application name filter.
 * @param app_name The application name to filter by (empty for no filter).
 */
auto LogFilterProxyModel::set_app_name_filter(const QString& app_name) -> void
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
auto LogFilterProxyModel::set_level_filter(const QSet<QString>& levels) -> void
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
auto LogFilterProxyModel::set_search_filter(const QString& search_text, const QString& field,
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
auto LogFilterProxyModel::filterAcceptsRow(int source_row,
                                           const QModelIndex& source_parent) const -> bool
{
    bool accepted = true;

    QModelIndex index_app = sourceModel()->index(source_row, LogModel::AppName, source_parent);
    QModelIndex index_level = sourceModel()->index(source_row, LogModel::Level, source_parent);
    QModelIndex index_message = sourceModel()->index(source_row, LogModel::Message, source_parent);

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

        if (m_search_field.compare("Message", Qt::CaseInsensitive) == 0)
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
