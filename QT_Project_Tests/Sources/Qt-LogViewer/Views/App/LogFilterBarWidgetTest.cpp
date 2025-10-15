#include "Qt-LogViewer/Views/App/LogFilterBarWidgetTest.h"

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QTest>
#include <QVector>

#include "Qt-LogViewer/Views/App/LogFilterWidget.h"
#include "Qt-LogViewer/Views/App/SearchBarWidget.h"

/**
 * @brief Sets up the test fixture for each test.
 */
void LogFilterBarWidgetTest::SetUp()
{
    m_widget = new LogFilterBarWidget();
}

/**
 * @brief Tears down the test fixture after each test.
 */
void LogFilterBarWidgetTest::TearDown()
{
    delete m_widget;
    m_widget = nullptr;
}

/**
 * @brief Helper to get the contained LogFilterWidget.
 * @return Pointer to LogFilterWidget, or nullptr.
 */
auto LogFilterBarWidgetTest::get_filter_widget() const -> LogFilterWidget*
{
    return m_widget->get_filter_widget();
}

/**
 * @brief Helper to get the contained SearchBarWidget.
 * @return Pointer to SearchBarWidget, or nullptr.
 */
auto LogFilterBarWidgetTest::get_search_bar_widget() const -> SearchBarWidget*
{
    return m_widget->get_search_bar_widget();
}

/**
 * @brief Helper to get the log level filter item widget for a given log level.
 * @param level The log level name (case-insensitive).
 * @return Pointer to LogLevelFilterItemWidget, or nullptr.
 */
auto LogFilterBarWidgetTest::get_log_level_item(const QString& level) const
    -> LogLevelFilterItemWidget*
{
    const QString normalized = level.trimmed().toLower();
    const auto items = get_filter_widget()->findChildren<LogLevelFilterItemWidget*>();
    for (auto* item: items)
    {
        if (item->get_log_level_name().trimmed().toLower() == normalized)
        {
            return item;
        }
    }
    return nullptr;
}

/**
 * @brief Tests construction and default state.
 */
TEST_F(LogFilterBarWidgetTest, ConstructionAndDefaultState)
{
    EXPECT_NE(m_widget, nullptr);
    EXPECT_NE(get_filter_widget(), nullptr);
    EXPECT_NE(get_search_bar_widget(), nullptr);
    EXPECT_EQ(m_widget->get_current_app_name(), "Show All Apps");
    EXPECT_TRUE(m_widget->get_current_log_levels().isEmpty());
    EXPECT_EQ(m_widget->get_search_text(), "");
    EXPECT_EQ(m_widget->get_search_field(), "All Fields");
    EXPECT_FALSE(m_widget->get_use_regex());
}

/**
 * @brief Tests setting and getting application names.
 */
TEST_F(LogFilterBarWidgetTest, SetAndGetAppNames)
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
TEST_F(LogFilterBarWidgetTest, SetAvailableLogLevelsCreatesItems)
{
    QVector<QString> levels = {"Trace", "Debug", "Info"};
    m_widget->set_available_log_levels(levels);
    EXPECT_NE(get_log_level_item("Trace"), nullptr);
    EXPECT_NE(get_log_level_item("Debug"), nullptr);
    EXPECT_NE(get_log_level_item("Info"), nullptr);
    EXPECT_EQ(get_filter_widget()->findChildren<LogLevelFilterItemWidget*>().size(), 3);
}

/**
 * @brief Tests setting and getting checked log levels.
 */
TEST_F(LogFilterBarWidgetTest, SetAndGetCheckedLogLevels)
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
TEST_F(LogFilterBarWidgetTest, SetLogLevelCounts)
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
TEST_F(LogFilterBarWidgetTest, ChangeAvailableLogLevelsRecreatesItems)
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

    const auto items = get_filter_widget()->findChildren<LogLevelFilterItemWidget*>();
    for (auto* item: items)
    {
        EXPECT_NE(item, trace_item_before);
        EXPECT_NE(item, debug_item_before);
    }
}

/**
 * @brief Tests that setting log levels with mixed case works.
 */
TEST_F(LogFilterBarWidgetTest, SetLogLevelsMixedCase)
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
TEST_F(LogFilterBarWidgetTest, SetLogLevelCountsMixedCase)
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
TEST_F(LogFilterBarWidgetTest, UncheckAllLogLevels)
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

/**
 * @brief Tests setting and getting search fields.
 */
TEST_F(LogFilterBarWidgetTest, SetAndGetSearchFields)
{
    QStringList fields = {"Message", "Source", "Thread"};
    m_widget->set_search_fields(fields);
    auto* search_bar = get_search_bar_widget();
    ASSERT_NE(search_bar, nullptr);
    auto* combo = search_bar->findChild<QComboBox*>("comboBoxSearchArea");
    ASSERT_NE(combo, nullptr);
    EXPECT_EQ(combo->count(), fields.size());
    EXPECT_EQ(combo->itemText(0), "Message");
    EXPECT_EQ(combo->itemText(1), "Source");
    EXPECT_EQ(combo->itemText(2), "Thread");
}

/**
 * @brief Tests setting and getting search placeholder text.
 */
TEST_F(LogFilterBarWidgetTest, SetSearchPlaceholder)
{
    QString placeholder = "Type to search...";
    m_widget->set_search_placeholder(placeholder);
    auto* search_bar = get_search_bar_widget();
    ASSERT_NE(search_bar, nullptr);
    auto* line_edit = search_bar->findChild<QLineEdit*>("lineEditSearch");
    ASSERT_NE(line_edit, nullptr);
    EXPECT_EQ(line_edit->placeholderText(), placeholder);
}

/**
 * @brief Tests setting and getting search text.
 */
TEST_F(LogFilterBarWidgetTest, SetAndGetSearchText)
{
    auto* search_bar = get_search_bar_widget();
    ASSERT_NE(search_bar, nullptr);
    auto* line_edit = search_bar->findChild<QLineEdit*>("lineEditSearch");
    ASSERT_NE(line_edit, nullptr);
    line_edit->setText("TestLogText");
    EXPECT_EQ(m_widget->get_search_text(), "TestLogText");
}

/**
 * @brief Tests setting and getting regex state.
 */
TEST_F(LogFilterBarWidgetTest, SetAndGetRegexState)
{
    auto* search_bar = get_search_bar_widget();
    ASSERT_NE(search_bar, nullptr);
    auto* regex_checkbox = search_bar->findChild<QCheckBox*>("checkBoxRegEx");
    ASSERT_NE(regex_checkbox, nullptr);
    regex_checkbox->setChecked(true);
    EXPECT_TRUE(m_widget->get_use_regex());
    regex_checkbox->setChecked(false);
    EXPECT_FALSE(m_widget->get_use_regex());
}
