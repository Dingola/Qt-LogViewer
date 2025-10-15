#include "Qt-LogViewer/Services/NumberFormatUtilsTest.h"

/**
 * @brief Sets up the test fixture for each test.
 */
void NumberFormatUtilsTest::SetUp() {}

/**
 * @brief Tears down the test fixture after each test.
 */
void NumberFormatUtilsTest::TearDown() {}

/**
 * @brief Tests abbreviated formatting for double values.
 */
TEST_F(NumberFormatUtilsTest, FormatNumberAbbreviated_Double)
{
    EXPECT_EQ(NumberFormatUtils::format_number_abbreviated(0.0), "0");
    EXPECT_EQ(NumberFormatUtils::format_number_abbreviated(999.0), "999");
    EXPECT_EQ(NumberFormatUtils::format_number_abbreviated(1000.0), "1.0K");
    EXPECT_EQ(NumberFormatUtils::format_number_abbreviated(1500.0), "1.5K");
    EXPECT_EQ(NumberFormatUtils::format_number_abbreviated(9999.0), "10.0K");
    EXPECT_EQ(NumberFormatUtils::format_number_abbreviated(10000.0), "10K");
    EXPECT_EQ(NumberFormatUtils::format_number_abbreviated(1000000.0), "1.0M");
    EXPECT_EQ(NumberFormatUtils::format_number_abbreviated(2500000.0), "2.5M");
    EXPECT_EQ(NumberFormatUtils::format_number_abbreviated(1000000000.0), "1.0B");
    EXPECT_EQ(NumberFormatUtils::format_number_abbreviated(1234567890.0), "1.2B");
    EXPECT_EQ(NumberFormatUtils::format_number_abbreviated(1000000000000.0), "1.0T");
}

/**
 * @brief Tests abbreviated formatting for integer values.
 */
TEST_F(NumberFormatUtilsTest, FormatNumberAbbreviated_Int)
{
    EXPECT_EQ(NumberFormatUtils::format_number_abbreviated(0), "0");
    EXPECT_EQ(NumberFormatUtils::format_number_abbreviated(999), "999");
    EXPECT_EQ(NumberFormatUtils::format_number_abbreviated(1000), "1.0K");
    EXPECT_EQ(NumberFormatUtils::format_number_abbreviated(1500), "1.5K");
    EXPECT_EQ(NumberFormatUtils::format_number_abbreviated(9999), "10.0K");
    EXPECT_EQ(NumberFormatUtils::format_number_abbreviated(10000), "10K");
    EXPECT_EQ(NumberFormatUtils::format_number_abbreviated(1000000), "1.0M");
    EXPECT_EQ(NumberFormatUtils::format_number_abbreviated(2500000), "2.5M");
    EXPECT_EQ(NumberFormatUtils::format_number_abbreviated(1000000000), "1.0B");
    EXPECT_EQ(NumberFormatUtils::format_number_abbreviated(1234567890), "1.2B");
}

/**
 * @brief Tests edge cases and rounding behavior.
 */
TEST_F(NumberFormatUtilsTest, FormatNumberAbbreviated_EdgeCases)
{
    EXPECT_EQ(NumberFormatUtils::format_number_abbreviated(999.9).toStdString(),
              std::string("999.9"));
    EXPECT_EQ(NumberFormatUtils::format_number_abbreviated(1000.1).toStdString(),
              std::string("1.0K"));
    EXPECT_EQ(NumberFormatUtils::format_number_abbreviated(-1000).toStdString(),
              std::string("-1.0K"));
    EXPECT_EQ(NumberFormatUtils::format_number_abbreviated(-1500000).toStdString(),
              std::string("-1.5M"));
}
