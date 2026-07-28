#include "Qt-LogViewer/Views/Shared/HoverRowDelegateTest.h"

#include "Qt-LogViewer/Models/LogModel.h"

/**
 * @file HoverRowDelegateTest.cpp
 * @brief Tests presentation-only search highlighting in HoverRowDelegate.
 */

/**
 * @brief Verifies that plain-text matches are found case-insensitively.
 */
TEST_F(HoverRowDelegateTest, HighlightsPlainTextCaseInsensitively)
{
    m_delegate.set_search_highlight(QStringLiteral("error"), SearchField::Message, false);

    const QVector<QPair<int, int>> ranges =
        m_delegate.get_highlight_ranges(QStringLiteral("Error and ERROR"), LogModel::Message);

    ASSERT_EQ(ranges.size(), 2);
    EXPECT_EQ(ranges.at(0), qMakePair(0, 5));
    EXPECT_EQ(ranges.at(1), qMakePair(10, 5));
}

/**
 * @brief Verifies that every regular-expression match is returned.
 */
TEST_F(HoverRowDelegateTest, HighlightsRegexMatches)
{
    m_delegate.set_search_highlight(QStringLiteral("err(or)?"), SearchField::Message, true);

    const QVector<QPair<int, int>> ranges =
        m_delegate.get_highlight_ranges(QStringLiteral("err Error"), LogModel::Message);

    ASSERT_EQ(ranges.size(), 2);
    EXPECT_EQ(ranges.at(0), qMakePair(0, 3));
    EXPECT_EQ(ranges.at(1), qMakePair(4, 5));
}

/**
 * @brief Verifies that highlighting is restricted to the selected field.
 */
TEST_F(HoverRowDelegateTest, HighlightsOnlySelectedField)
{
    m_delegate.set_search_highlight(QStringLiteral("error"), SearchField::Message, false);

    EXPECT_TRUE(
        m_delegate.get_highlight_ranges(QStringLiteral("ERROR"), LogModel::Level).isEmpty());

    EXPECT_FALSE(
        m_delegate.get_highlight_ranges(QStringLiteral("ERROR"), LogModel::Message).isEmpty());
}

/**
 * @brief Verifies that an all-fields search highlights matches in different columns.
 */
TEST_F(HoverRowDelegateTest, HighlightsAllFields)
{
    m_delegate.set_search_highlight(QStringLiteral("app"), SearchField::AllFields, false);

    EXPECT_FALSE(
        m_delegate.get_highlight_ranges(QStringLiteral("MyApp"), LogModel::AppName).isEmpty());

    EXPECT_FALSE(
        m_delegate.get_highlight_ranges(QStringLiteral("AppMessage"), LogModel::Message).isEmpty());
}

/**
 * @brief Verifies that an invalid regular expression produces no highlight ranges.
 */
TEST_F(HoverRowDelegateTest, InvalidRegexProducesNoRanges)
{
    m_delegate.set_search_highlight(QStringLiteral("("), SearchField::Message, true);

    EXPECT_TRUE(
        m_delegate.get_highlight_ranges(QStringLiteral("anything"), LogModel::Message).isEmpty());
}

/**
 * @brief Verifies that an empty search produces no highlight ranges.
 */
TEST_F(HoverRowDelegateTest, EmptySearchProducesNoRanges)
{
    m_delegate.set_search_highlight(QString(), SearchField::AllFields, false);

    EXPECT_TRUE(
        m_delegate.get_highlight_ranges(QStringLiteral("anything"), LogModel::Message).isEmpty());
}

/**
 * @brief Verifies that the Count sentinel never enables highlighting.
 */
TEST_F(HoverRowDelegateTest, CountFieldProducesNoRanges)
{
    m_delegate.set_search_highlight(QStringLiteral("anything"), SearchField::Count, false);

    EXPECT_TRUE(
        m_delegate.get_highlight_ranges(QStringLiteral("anything"), LogModel::Message).isEmpty());
}
