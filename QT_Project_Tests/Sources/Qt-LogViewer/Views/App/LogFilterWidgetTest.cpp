#include "Qt-LogViewer/Views/App/LogFilterWidgetTest.h"

#include <QLabel>
#include <QTest>

/**
 * @brief Sets up the test fixture for each test.
 */
void LogFilterWidgetTest::SetUp()
{
    m_widget = new LogFilterWidget();
}

/**
 * @brief Tears down the test fixture after each test.
 */
void LogFilterWidgetTest::TearDown()
{
    delete m_widget;
    m_widget = nullptr;
}

/**
 * @brief Helper to get the log level filter item widget for a given log level.
 * @param level The log level name (case-insensitive).
 * @return Pointer to the LogLevelFilterItemWidget, or nullptr.
 */
auto LogFilterWidgetTest::get_log_level_item(const QString& level) const
    -> LogLevelFilterItemWidget*
{
    const QString normalized = level.trimmed().toLower();
    const auto items = m_widget->findChildren<LogLevelFilterItemWidget*>();
    LogLevelFilterItemWidget* result = nullptr;

    for (auto* item: items)
    {
        if (item->get_log_level_name().trimmed().toLower() == normalized)
        {
            result = item;
        }
    }

    return result;
}

/**
 * @brief Tests construction and default state.
 */
TEST_F(LogFilterWidgetTest, ConstructionAndDefaultState)
{
    EXPECT_NE(m_widget, nullptr);
    EXPECT_EQ(m_widget->get_current_app_name(), "Show All Apps");
    EXPECT_TRUE(m_widget->get_current_log_levels().isEmpty());
}

/**
 * @brief Tests setting and getting application names.
 */
TEST_F(LogFilterWidgetTest, SetAndGetAppNames)
{
    QSet<QString> app_names = {"AppA", "AppB"};
    m_widget->set_app_names(app_names);
    EXPECT_EQ(m_widget->get_current_app_name(), "Show All Apps");
    m_widget->set_current_app_name_filter("AppB");
    EXPECT_EQ(m_widget->get_current_app_name(), "AppB");
}

/**
 * @brief Tests setting available log levels and their widgets.
 */
TEST_F(LogFilterWidgetTest, SetAvailableLogLevelsCreatesItems)
{
    QVector<QString> levels = {"Trace", "Debug", "Info"};
    m_widget->set_available_log_levels(levels);
    EXPECT_NE(get_log_level_item("Trace"), nullptr);
    EXPECT_NE(get_log_level_item("Debug"), nullptr);
    EXPECT_NE(get_log_level_item("Info"), nullptr);
    EXPECT_EQ(m_widget->findChildren<LogLevelFilterItemWidget*>().size(), 3);
}

/**
 * @brief Tests setting and getting checked log levels.
 */
TEST_F(LogFilterWidgetTest, SetAndGetCheckedLogLevels)
{
    QVector<QString> levels = {"Trace", "Debug", "Info"};
    m_widget->set_available_log_levels(levels);
    QSet<QString> checked = {"trace", "info"};
    m_widget->set_log_levels(checked);
    QSet<QString> result = m_widget->get_current_log_levels();
    EXPECT_TRUE(result.contains("trace"));
    EXPECT_TRUE(result.contains("info"));
    EXPECT_FALSE(result.contains("debug"));
}

/**
 * @brief Tests setting log level counts.
 */
TEST_F(LogFilterWidgetTest, SetLogLevelCounts)
{
    QVector<QString> levels = {"Trace", "Debug", "Info"};
    m_widget->set_available_log_levels(levels);

    QMap<QString, int> counts;
    counts["Trace"] = 5;
    counts["Debug"] = 10;
    counts["Info"] = 0;
    m_widget->set_log_level_counts(counts);

    auto* trace_item = get_log_level_item("Trace");
    auto* debug_item = get_log_level_item("Debug");
    auto* info_item = get_log_level_item("Info");
    ASSERT_NE(trace_item, nullptr);
    ASSERT_NE(debug_item, nullptr);
    ASSERT_NE(info_item, nullptr);

    QLabel* trace_count = trace_item->findChild<QLabel*>("labelCount");
    QLabel* debug_count = debug_item->findChild<QLabel*>("labelCount");
    QLabel* info_count = info_item->findChild<QLabel*>("labelCount");
    ASSERT_NE(trace_count, nullptr);
    ASSERT_NE(debug_count, nullptr);
    ASSERT_NE(info_count, nullptr);

    EXPECT_EQ(trace_count->text(), "5");
    EXPECT_EQ(debug_count->text(), "10");
    EXPECT_EQ(info_count->text(), "0");
}

/**
 * @brief Tests that log level items are cleared and recreated when available log levels change.
 */
TEST_F(LogFilterWidgetTest, ChangeAvailableLogLevelsRecreatesItems)
{
    QVector<QString> levels1 = {"Trace", "Debug"};
    QVector<QString> levels2 = {"Info", "Warning"};
    m_widget->set_available_log_levels(levels1);

    auto* trace_item_before = get_log_level_item("Trace");
    auto* debug_item_before = get_log_level_item("Debug");
    ASSERT_NE(trace_item_before, nullptr);
    ASSERT_NE(debug_item_before, nullptr);

    m_widget->set_available_log_levels(levels2);
    QTest::qWait(0);

    auto* trace_item_after = get_log_level_item("Trace");
    auto* debug_item_after = get_log_level_item("Debug");
    auto* info_item_after = get_log_level_item("Info");
    auto* warning_item_after = get_log_level_item("Warning");

    EXPECT_EQ(trace_item_after, nullptr);
    EXPECT_EQ(debug_item_after, nullptr);
    EXPECT_NE(info_item_after, nullptr);
    EXPECT_NE(warning_item_after, nullptr);

    // Ensure the old pointers are not present in the new widget tree
    const auto items = m_widget->findChildren<LogLevelFilterItemWidget*>();
    for (auto* item: items)
    {
        EXPECT_NE(item, trace_item_before);
        EXPECT_NE(item, debug_item_before);
    }
}

/**
 * @brief Tests that setting log levels with mixed case works.
 */
TEST_F(LogFilterWidgetTest, SetLogLevelsMixedCase)
{
    QVector<QString> levels = {"Trace", "Debug", "Info"};
    m_widget->set_available_log_levels(levels);
    QSet<QString> checked = {"TrAcE", "dEbUg"};
    m_widget->set_log_levels(checked);
    QSet<QString> result = m_widget->get_current_log_levels();
    EXPECT_TRUE(result.contains("trace"));
    EXPECT_TRUE(result.contains("debug"));
    EXPECT_FALSE(result.contains("info"));
}

/**
 * @brief Tests that setting log level counts with mixed case works.
 */
TEST_F(LogFilterWidgetTest, SetLogLevelCountsMixedCase)
{
    QVector<QString> levels = {"Trace", "Debug"};
    m_widget->set_available_log_levels(levels);
    QMap<QString, int> counts;
    counts["TrAcE"] = 7;
    counts["dEbUg"] = 3;
    m_widget->set_log_level_counts(counts);
    auto* trace_item = get_log_level_item("Trace");
    auto* debug_item = get_log_level_item("Debug");
    ASSERT_NE(trace_item, nullptr);
    ASSERT_NE(debug_item, nullptr);
}

/**
 * @brief Tests that all log level items are unchecked after set_log_levels({}).
 */
TEST_F(LogFilterWidgetTest, UncheckAllLogLevels)
{
    QVector<QString> levels = {"Trace", "Debug"};
    m_widget->set_available_log_levels(levels);
    m_widget->set_log_levels({});
    auto* trace_item = get_log_level_item("Trace");
    auto* debug_item = get_log_level_item("Debug");
    ASSERT_NE(trace_item, nullptr);
    ASSERT_NE(debug_item, nullptr);
    EXPECT_FALSE(trace_item->get_checked());
    EXPECT_FALSE(debug_item->get_checked());
}
