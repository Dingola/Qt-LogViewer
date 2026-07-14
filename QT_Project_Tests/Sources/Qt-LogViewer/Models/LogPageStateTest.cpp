#include "Qt-LogViewer/Models/LogPageStateTest.h"

#include "Qt-LogViewer/Models/LogPageState.h"

/**
 * @brief Verifies the default pagination state.
 */
TEST_F(LogPageStateTest, UsesDefaultPagingValues)
{
    const LogPageState state;

    EXPECT_EQ(state.get_page_size(), 25);
    EXPECT_EQ(state.get_current_page(), 1);
    EXPECT_EQ(state.get_total_entries(), 0);
    EXPECT_EQ(state.get_total_pages(), 1);
    EXPECT_EQ(state.get_offset(), 0);
}

/**
 * @brief Verifies page-count calculation for complete and partial pages.
 */
TEST_F(LogPageStateTest, CalculatesTotalPages)
{
    LogPageState state;
    state.set_page_size(25);

    state.set_total_entries(25);
    EXPECT_EQ(state.get_total_pages(), 1);

    state.set_total_entries(26);
    EXPECT_EQ(state.get_total_pages(), 2);

    state.set_total_entries(100);
    EXPECT_EQ(state.get_total_pages(), 4);

    state.set_total_entries(101);
    EXPECT_EQ(state.get_total_pages(), 5);
}

/**
 * @brief Verifies offset calculation for one-based page numbers.
 */
TEST_F(LogPageStateTest, CalculatesCurrentPageOffset)
{
    LogPageState state;
    state.set_page_size(25);
    state.set_total_entries(100);
    state.set_current_page(3);

    EXPECT_EQ(state.get_current_page(), 3);
    EXPECT_EQ(state.get_offset(), 50);
}

/**
 * @brief Verifies that page numbers remain within the available range.
 */
TEST_F(LogPageStateTest, RestrictsCurrentPageToAvailableRange)
{
    LogPageState state;
    state.set_page_size(25);
    state.set_total_entries(60);

    state.set_current_page(0);
    EXPECT_EQ(state.get_current_page(), 1);

    state.set_current_page(10);
    EXPECT_EQ(state.get_current_page(), 3);
}

/**
 * @brief Verifies that reducing the result count adjusts the current page.
 */
TEST_F(LogPageStateTest, AdjustsPageWhenResultCountShrinks)
{
    LogPageState state;
    state.set_page_size(25);
    state.set_total_entries(100);
    state.set_current_page(4);

    state.set_total_entries(30);

    EXPECT_EQ(state.get_current_page(), 2);
    EXPECT_EQ(state.get_offset(), 25);
}

/**
 * @brief Verifies that changing the query resets its result state.
 */
TEST_F(LogPageStateTest, ResetsPagingWhenQueryChanges)
{
    LogPageState state;
    state.set_total_entries(100);
    state.set_current_page(3);

    LogQuery query;
    query.view_id = QUuid::createUuid();
    query.search_text = QStringLiteral("network");

    state.set_query(query);

    EXPECT_EQ(state.get_query().view_id, query.view_id);
    EXPECT_EQ(state.get_query().search_text, QStringLiteral("network"));
    EXPECT_EQ(state.get_current_page(), 1);
    EXPECT_EQ(state.get_total_entries(), 0);
    EXPECT_EQ(state.get_page_size(), 25);
}

/**
 * @brief Verifies that invalid page sizes are ignored.
 */
TEST_F(LogPageStateTest, IgnoresInvalidPageSize)
{
    LogPageState state;

    state.set_page_size(0);
    EXPECT_EQ(state.get_page_size(), 25);

    state.set_page_size(-10);
    EXPECT_EQ(state.get_page_size(), 25);
}
