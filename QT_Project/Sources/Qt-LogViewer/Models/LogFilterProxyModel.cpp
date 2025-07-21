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
        recalc_paging();
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
        recalc_paging();
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

        recalc_paging();
    }
}

/**
 * @brief Enables or disables paging.
 * @param enabled True to enable paging, false to show all results.
 */
auto LogFilterProxyModel::set_paging_enabled(bool enabled) -> void
{
    if (m_paging_enabled != enabled)
    {
        m_paging_enabled = enabled;
        recalc_paging();
    }
}

/**
 * @brief Sets the number of items per page.
 * @param size The number of items per page.
 */
auto LogFilterProxyModel::set_page_size(int size) -> void
{
    if (size > 0 && m_page_size != size)
    {
        m_page_size = size;
        recalc_paging();
    }
}

/**
 * @brief Sets the current page (1-based).
 * @param page The page number to display.
 */
auto LogFilterProxyModel::set_current_page(int page) -> void
{
    if (page < 1)
    {
        page = 1;
    }

    if (m_current_page != page)
    {
        m_current_page = page;
        recalc_paging();
    }
}

/**
 * @brief Returns the current page (1-based).
 * @return The current page index.
 */
auto LogFilterProxyModel::get_current_page() const -> int
{
    return m_current_page;
}

/**
 * @brief Returns the total number of pages based on the current filter and items per page.
 * @return The total number of pages.
 */
auto LogFilterProxyModel::get_total_pages() const -> int
{
    return m_total_pages;
}

/**
 * @brief Returns the number of items to display per page.
 * @return The number of items per page.
 */
auto LogFilterProxyModel::get_items_per_page() const -> int
{
    return m_page_size;
}

/**
 * @brief Returns whether paging is enabled.
 * @return True if paging is enabled, false otherwise.
 */
auto LogFilterProxyModel::is_paging_enabled() const -> bool
{
    return m_paging_enabled;
}

/**
 * @brief Sets the source model for this proxy model.
 * @param sourceModel The source model to filter.
 *
 * Connects to the source model's signals to recalculate paging when the model changes.
 */
void LogFilterProxyModel::setSourceModel(QAbstractItemModel* sourceModel)
{
    QSortFilterProxyModel::setSourceModel(sourceModel);

    if (sourceModel)
    {
        connect(sourceModel, &QAbstractItemModel::rowsInserted, this, [this] { recalc_paging(); });
        connect(sourceModel, &QAbstractItemModel::rowsRemoved, this, [this] { recalc_paging(); });
        connect(sourceModel, &QAbstractItemModel::modelReset, this, [this] { recalc_paging(); });
        connect(sourceModel, &QAbstractItemModel::dataChanged, this, [this] { recalc_paging(); });
    }
}

/**
 * @brief Determines whether the given row should be included in the filtered model and current
 * page.
 * @param source_row The row in the source model.
 * @param source_parent The parent index in the source model.
 * @return True if the row matches all filters and is in the current page, false otherwise.
 */
auto LogFilterProxyModel::filterAcceptsRow(int source_row,
                                           const QModelIndex& source_parent) const -> bool
{
    bool accepted = row_passes_filter(source_row, source_parent);

    if (accepted && m_paging_enabled)
    {
        int filtered_index = 0;
        int row_count = sourceModel()->rowCount(source_parent);

        for (int row = 0; row < source_row; ++row)
        {
            if (row_passes_filter(row, source_parent))
            {
                ++filtered_index;
            }
        }

        int min = (m_current_page - 1) * m_page_size;
        int max = min + m_page_size;

        if (!(filtered_index >= min && filtered_index < max))
        {
            accepted = false;
        }
    }

    return accepted;
}

/**
 * @brief Recalculates paging and invalidates the filter.
 */
auto LogFilterProxyModel::recalc_paging() -> void
{
    int filtered_count = 0;
    int row_count = sourceModel() ? sourceModel()->rowCount() : 0;

    for (int row = 0; row < row_count; ++row)
    {
        if (row_passes_filter(row, QModelIndex()))
        {
            ++filtered_count;
        }
    }

    if (m_paging_enabled)
    {
        m_total_pages = (filtered_count + m_page_size - 1) / m_page_size;

        if (m_total_pages < 1)
        {
            m_total_pages = 1;
        }
        if (m_current_page > m_total_pages)
        {
            m_current_page = m_total_pages;
        }
    }
    else
    {
        m_total_pages = 1;
        m_current_page = 1;
    }

    QSortFilterProxyModel::invalidateFilter();
}

/**
 * @brief Determines whether the given row passes the filter.
 * @param row The row index in the proxy model.
 * @param parent The parent index in the proxy model.
 * @return True if the row passes the filter, false otherwise.
 */
auto LogFilterProxyModel::row_passes_filter(int row, const QModelIndex& parent) const -> bool
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
