#include "Qt-LogViewer/Views/App/LogLevelPieChartWidgetTest.h"

#include <QHBoxLayout>
#include <QTest>
#include <QVBoxLayout>

/**
 * @brief Sets up the test fixture for each test.
 */
void LogLevelPieChartWidgetTest::SetUp()
{
    m_widget = new LogLevelPieChartWidget();
}

/**
 * @brief Tears down the test fixture after each test.
 */
void LogLevelPieChartWidgetTest::TearDown()
{
    delete m_widget;
    m_widget = nullptr;
}

/**
 * @brief Helper to get the row widget for a log level.
 * @param level The log level name (e.g., "trace").
 * @return Pointer to the QWidget for the row, or nullptr.
 */
auto LogLevelPieChartWidgetTest::get_row_widget(const QString& level) const -> QWidget*
{
    QWidget* result = nullptr;
    const auto row_widgets = m_widget->findChildren<QWidget*>("logLevelRowWidget");

    for (QWidget* row: row_widgets)
    {
        auto* name_label = row->findChild<QLabel*>("logLevelNameLabel");

        if (name_label != nullptr &&
            name_label->text().compare(level.toUpper(), Qt::CaseInsensitive) == 0)
        {
            result = row;
        }
    }

    return result;
}

/**
 * @brief Helper to get the percent label for a log level.
 * @param level The log level name (e.g., "trace").
 * @return Pointer to the QLabel showing the percent, or nullptr.
 */
auto LogLevelPieChartWidgetTest::get_percent_label(const QString& level) const -> QLabel*
{
    QLabel* result = nullptr;
    QWidget* row = get_row_widget(level);

    if (row != nullptr)
    {
        result = row->findChild<QLabel*>("logLevelPercentLabel");
    }

    return result;
}

/**
 * @brief Helper to get the name label for a log level.
 * @param level The log level name (e.g., "trace").
 * @return Pointer to the QLabel showing the name, or nullptr.
 */
auto LogLevelPieChartWidgetTest::get_name_label(const QString& level) const -> QLabel*
{
    QLabel* result = nullptr;
    QWidget* row = get_row_widget(level);

    if (row != nullptr)
    {
        result = row->findChild<QLabel*>("logLevelNameLabel");
    }

    return result;
}

/**
 * @brief Helper to get the color label for a log level.
 * @param level The log level name (e.g., "trace").
 * @return Pointer to the QLabel showing the color, or nullptr.
 */
auto LogLevelPieChartWidgetTest::get_color_label(const QString& level) const -> QLabel*
{
    QLabel* result = nullptr;
    QWidget* row = get_row_widget(level);

    if (row != nullptr)
    {
        result = row->findChild<QLabel*>("logLevelColorSquare");
    }

    return result;
}

/**
 * @brief Tests construction and default colors.
 */
TEST_F(LogLevelPieChartWidgetTest, ConstructionAndDefaultColors)
{
    EXPECT_NE(m_widget, nullptr);
    EXPECT_EQ(m_widget->get_trace_color(), QColor("#b0bec5"));
    EXPECT_EQ(m_widget->get_debug_color(), QColor("#66bb6a"));
    EXPECT_EQ(m_widget->get_info_color(), QColor("#42a5f5"));
    EXPECT_EQ(m_widget->get_warning_color(), QColor("#ffb300"));
    EXPECT_EQ(m_widget->get_error_color(), QColor("#ef5350"));
    EXPECT_EQ(m_widget->get_fatal_color(), QColor("#ff1744"));
}

/**
 * @brief Tests setting and getting color properties.
 */
TEST_F(LogLevelPieChartWidgetTest, SetAndGetColorProperties)
{
    QColor new_trace("#123456");
    m_widget->set_trace_color(new_trace);
    EXPECT_EQ(m_widget->get_trace_color(), new_trace);

    QColor new_debug("#654321");
    m_widget->set_debug_color(new_debug);
    EXPECT_EQ(m_widget->get_debug_color(), new_debug);

    QColor new_info("#abcdef");
    m_widget->set_info_color(new_info);
    EXPECT_EQ(m_widget->get_info_color(), new_info);

    QColor new_warning("#fedcba");
    m_widget->set_warning_color(new_warning);
    EXPECT_EQ(m_widget->get_warning_color(), new_warning);

    QColor new_error("#112233");
    m_widget->set_error_color(new_error);
    EXPECT_EQ(m_widget->get_error_color(), new_error);

    QColor new_fatal("#332211");
    m_widget->set_fatal_color(new_fatal);
    EXPECT_EQ(m_widget->get_fatal_color(), new_fatal);
}

/**
 * @brief Tests setting and getting inner radius percent and segment gap angle.
 */
TEST_F(LogLevelPieChartWidgetTest, SetAndGetRadiusAndGap)
{
    m_widget->set_inner_radius_percent(80);
    EXPECT_EQ(m_widget->get_inner_radius_percent(), 80);

    m_widget->set_segment_gap_angle(6);
    EXPECT_EQ(m_widget->get_segment_gap_angle(), 6);
}

/**
 * @brief Tests setting log level counts and percent calculation.
 */
TEST_F(LogLevelPieChartWidgetTest, SetLogLevelCountsAndPercentCalculation)
{
    QMap<QString, int> counts;
    counts["Trace"] = 10;
    counts["Debug"] = 20;
    counts["Info"] = 30;
    counts["Warning"] = 40;
    counts["Error"] = 0;
    counts["Fatal"] = 0;

    m_widget->set_log_level_counts(counts);

    // Percent labels
    auto* trace_label = get_percent_label("trace");
    auto* debug_label = get_percent_label("debug");
    auto* info_label = get_percent_label("info");
    auto* warning_label = get_percent_label("warning");
    EXPECT_NE(trace_label, nullptr);
    EXPECT_NE(debug_label, nullptr);
    EXPECT_NE(info_label, nullptr);
    EXPECT_NE(warning_label, nullptr);

    EXPECT_EQ(trace_label->text(), "10.0%");
    EXPECT_EQ(debug_label->text(), "20.0%");
    EXPECT_EQ(info_label->text(), "30.0%");
    EXPECT_EQ(warning_label->text(), "40.0%");

    // Error and Fatal should not be present
    EXPECT_EQ(get_percent_label("error"), nullptr);
    EXPECT_EQ(get_percent_label("fatal"), nullptr);
}

/**
 * @brief Tests that percent calculation works with non-integer results.
 */
TEST_F(LogLevelPieChartWidgetTest, PercentCalculationWithDecimals)
{
    QMap<QString, int> counts;
    counts["Trace"] = 1;
    counts["Debug"] = 2;
    counts["Info"] = 3;

    m_widget->set_log_level_counts(counts);

    auto* trace_label = get_percent_label("trace");
    auto* debug_label = get_percent_label("debug");
    auto* info_label = get_percent_label("info");

    EXPECT_EQ(trace_label->text(), "16.7%");
    EXPECT_EQ(debug_label->text(), "33.3%");
    EXPECT_EQ(info_label->text(), "50.0%");
}

/**
 * @brief Tests that widget hides rows when total is zero.
 */
TEST_F(LogLevelPieChartWidgetTest, NoRowsWhenTotalZero)
{
    QMap<QString, int> counts;
    counts["Trace"] = 0;
    counts["Debug"] = 0;
    counts["Info"] = 0;
    counts["Warning"] = 0;
    counts["Error"] = 0;
    counts["Fatal"] = 0;

    m_widget->set_log_level_counts(counts);

    // The rows should not be present
    auto row_widgets = m_widget->findChildren<QWidget*>("logLevelRowWidget");
    EXPECT_TRUE(row_widgets.isEmpty());

    // The levels widget should be hidden
    auto* levels_widget = m_widget->findChild<QWidget*>("logLevelRowsWidget");
    ASSERT_NE(levels_widget, nullptr);
    EXPECT_FALSE(levels_widget->isVisible());
}

/**
 * @brief Tests that color square updates when color is changed.
 */
TEST_F(LogLevelPieChartWidgetTest, ColorSquareUpdates)
{
    QMap<QString, int> counts;
    counts["Trace"] = 5;
    m_widget->set_log_level_counts(counts);

    QColor new_color("#123456");
    m_widget->set_trace_color(new_color);

    auto* color_label = get_color_label("trace");
    ASSERT_NE(color_label, nullptr);

    EXPECT_TRUE(color_label->styleSheet().contains(new_color.name()));
}

/**
 * @brief Tests that log level names are shown in uppercase.
 */
TEST_F(LogLevelPieChartWidgetTest, LogLevelNamesUppercase)
{
    QMap<QString, int> counts;
    counts["Info"] = 1;
    m_widget->set_log_level_counts(counts);

    auto* name_label = get_name_label("info");
    ASSERT_NE(name_label, nullptr);

    EXPECT_EQ(name_label->text(), "INFO");
}

/**
 * @brief Tests that setting counts with mixed case log level names works.
 */
TEST_F(LogLevelPieChartWidgetTest, MixedCaseLogLevelNames)
{
    QMap<QString, int> counts;
    counts["TrAcE"] = 7;
    counts["DeBuG"] = 3;
    m_widget->set_log_level_counts(counts);

    auto* trace_label = get_percent_label("trace");
    auto* debug_label = get_percent_label("debug");
    EXPECT_NE(trace_label, nullptr);
    EXPECT_NE(debug_label, nullptr);
}

/**
 * @brief Tests that updating log level counts updates rows correctly.
 */
TEST_F(LogLevelPieChartWidgetTest, UpdateLogLevelCountsUpdatesRows)
{
    QMap<QString, int> counts1;
    counts1["Trace"] = 5;
    counts1["Debug"] = 5;
    m_widget->set_log_level_counts(counts1);

    auto row_widgets1 = m_widget->findChildren<QWidget*>("logLevelRowWidget");
    EXPECT_EQ(row_widgets1.size(), 2);

    QMap<QString, int> counts2;
    counts2["Info"] = 10;
    m_widget->set_log_level_counts(counts2);

    auto row_widgets2 = m_widget->findChildren<QWidget*>("logLevelRowWidget");
    EXPECT_EQ(row_widgets2.size(), 1);
    EXPECT_NE(get_percent_label("info"), nullptr);
    EXPECT_EQ(get_percent_label("trace"), nullptr);
    EXPECT_EQ(get_percent_label("debug"), nullptr);
}

/**
 * @brief Tests that only one log level present is handled correctly.
 */
TEST_F(LogLevelPieChartWidgetTest, SingleLogLevelOnly)
{
    QMap<QString, int> counts;
    counts["Info"] = 100;

    m_widget->set_log_level_counts(counts);

    auto row_widgets = m_widget->findChildren<QWidget*>("logLevelRowWidget");
    EXPECT_EQ(row_widgets.size(), 1);
    auto* info_label = get_percent_label("info");
    ASSERT_NE(info_label, nullptr);
    EXPECT_EQ(info_label->text(), "100.0%");
}
