#include "Qt-LogViewer/Models/LogQueryTest.h"

#include "Qt-LogViewer/Models/LogQuery.h"

/**
 * @brief Verifies that the newest timestamp is displayed first by default.
 */
TEST_F(LogQueryTest, UsesNewestTimestampFirstByDefault)
{
    const LogQuery query;

    EXPECT_EQ(query.sort_field, LogField::Timestamp);
    EXPECT_EQ(query.sort_order, Qt::DescendingOrder);
}

/**
 * @brief Verifies that a default query does not restrict the result set.
 */
TEST_F(LogQueryTest, UsesUnrestrictedFiltersByDefault)
{
    const LogQuery query;

    EXPECT_TRUE(query.view_id.isNull());
    EXPECT_TRUE(query.app_name.isEmpty());
    EXPECT_TRUE(query.log_levels.isEmpty());
    EXPECT_TRUE(query.search_text.isEmpty());
    EXPECT_TRUE(query.search_fields.isEmpty());
    EXPECT_FALSE(query.use_regex);
    EXPECT_TRUE(query.show_only_file.isEmpty());
    EXPECT_TRUE(query.hidden_files.isEmpty());
}

/**
 * @brief Verifies that fields supplied by custom parsers can be searched.
 */
TEST_F(LogQueryTest, AcceptsDynamicSearchFields)
{
    LogQuery query;

    query.search_fields.insert(QStringLiteral("thread_id"));
    query.search_fields.insert(QStringLiteral("request_id"));

    EXPECT_TRUE(query.search_fields.contains(QStringLiteral("thread_id")));
    EXPECT_TRUE(query.search_fields.contains(QStringLiteral("request_id")));
}

/**
 * @brief Verifies that sorting is not restricted to built-in fields.
 */
TEST_F(LogQueryTest, AcceptsDynamicSortField)
{
    LogQuery query;

    query.sort_field = QStringLiteral("duration_ms");
    query.sort_order = Qt::AscendingOrder;

    EXPECT_EQ(query.sort_field, QStringLiteral("duration_ms"));
    EXPECT_EQ(query.sort_order, Qt::AscendingOrder);
}

/**
 * @brief Verifies that insertion order is available as a virtual sort field.
 */
TEST_F(LogQueryTest, SupportsInsertionOrderSorting)
{
    LogQuery query;

    query.sort_field = LogField::InsertionOrder;

    EXPECT_EQ(query.sort_field, QStringLiteral("_insertion_order"));
}
