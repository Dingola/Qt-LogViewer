#pragma once

#include <QtTypes>

#include "Qt-LogViewer/Models/LogQuery.h"

/**
 * @file LogPageState.h
 * @brief Declares the paging state for a database-backed log query.
 */

/**
 * @class LogPageState
 * @brief Stores a log query and its current pagination state.
 */
class LogPageState
{
    public:
        /**
         * @brief Constructs a page state with page 1 and 25 entries per page.
         */
        LogPageState() = default;

        /**
         * @brief Sets the query represented by this state.
         * @param query Query describing the result set.
         */
        auto set_query(const LogQuery& query) -> void;

        /**
         * @brief Returns the current query.
         * @return Query describing the result set.
         */
        [[nodiscard]] auto get_query() const -> const LogQuery&;

        /**
         * @brief Sets the number of entries displayed on one page.
         * @param page_size Positive number of entries per page.
         */
        auto set_page_size(qsizetype page_size) -> void;

        /**
         * @brief Returns the number of entries displayed on one page.
         * @return Positive page size.
         */
        [[nodiscard]] auto get_page_size() const -> qsizetype;

        /**
         * @brief Sets the current one-based page number.
         * @param page Requested page number.
         */
        auto set_current_page(qsizetype page) -> void;

        /**
         * @brief Returns the current one-based page number.
         * @return Current page number.
         */
        [[nodiscard]] auto get_current_page() const -> qsizetype;

        /**
         * @brief Sets the total number of matching entries.
         * @param total_entries Non-negative entry count.
         */
        auto set_total_entries(qsizetype total_entries) -> void;

        /**
         * @brief Returns the total number of matching entries.
         * @return Non-negative entry count.
         */
        [[nodiscard]] auto get_total_entries() const -> qsizetype;

        /**
         * @brief Returns the number of pages in the result set.
         * @return Total page count, with one page representing an empty result.
         */
        [[nodiscard]] auto get_total_pages() const -> qsizetype;

        /**
         * @brief Returns the database offset of the current page.
         * @return Zero-based entry offset.
         */
        [[nodiscard]] auto get_offset() const -> qsizetype;

    private:
        /**
         * @brief Restricts the current page to the available page range.
         */
        auto validate_current_page() -> void;

    private:
        LogQuery m_query;
        qsizetype m_page_size{25};
        qsizetype m_current_page{1};
        qsizetype m_total_entries{0};
};
