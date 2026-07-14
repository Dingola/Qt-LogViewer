/**
 * @file LogPageState.cpp
 * @brief Implements the paging state for a database-backed log query.
 */

#include "Qt-LogViewer/Models/LogPageState.h"

#include <algorithm>

/**
 * @brief Sets the query represented by this state.
 * @param query Query describing the result set.
 */
auto LogPageState::set_query(const LogQuery& query) -> void
{
    m_query = query;
    m_current_page = 1;
    m_total_entries = 0;
}

/**
 * @brief Returns the current query.
 * @return Query describing the result set.
 */
auto LogPageState::get_query() const -> const LogQuery&
{
    return m_query;
}

/**
 * @brief Sets the number of entries displayed on one page.
 * @param page_size Positive number of entries per page.
 */
auto LogPageState::set_page_size(qsizetype page_size) -> void
{
    if (page_size > 0)
    {
        m_page_size = page_size;
        validate_current_page();
    }
}

/**
 * @brief Returns the number of entries displayed on one page.
 * @return Positive page size.
 */
auto LogPageState::get_page_size() const -> qsizetype
{
    return m_page_size;
}

/**
 * @brief Sets the current one-based page number.
 * @param page Requested page number.
 */
auto LogPageState::set_current_page(qsizetype page) -> void
{
    m_current_page = page;
    validate_current_page();
}

/**
 * @brief Returns the current one-based page number.
 * @return Current page number.
 */
auto LogPageState::get_current_page() const -> qsizetype
{
    return m_current_page;
}

/**
 * @brief Sets the total number of matching entries.
 * @param total_entries Non-negative entry count.
 */
auto LogPageState::set_total_entries(qsizetype total_entries) -> void
{
    m_total_entries = std::max<qsizetype>(0, total_entries);
    validate_current_page();
}

/**
 * @brief Returns the total number of matching entries.
 * @return Non-negative entry count.
 */
auto LogPageState::get_total_entries() const -> qsizetype
{
    return m_total_entries;
}

/**
 * @brief Returns the number of pages in the result set.
 * @return Total page count, with one page representing an empty result.
 */
auto LogPageState::get_total_pages() const -> qsizetype
{
    qsizetype total_pages = 1;

    if (m_total_entries > 0)
    {
        total_pages = m_total_entries / m_page_size;

        if ((m_total_entries % m_page_size) != 0)
        {
            ++total_pages;
        }
    }

    return total_pages;
}

/**
 * @brief Returns the database offset of the current page.
 * @return Zero-based entry offset.
 */
auto LogPageState::get_offset() const -> qsizetype
{
    return (m_current_page - 1) * m_page_size;
}

/**
 * @brief Restricts the current page to the available page range.
 */
auto LogPageState::validate_current_page() -> void
{
    m_current_page = std::clamp(m_current_page, qsizetype{1}, get_total_pages());
}
