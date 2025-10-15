#include "Qt-LogViewer/Views/App/LogLevelFilterItemWidgetTest.h"

#include <QSignalSpy>
#include <QTest>

#include "Qt-LogViewer/Views/App/LogLevelFilterItemWidget.h"

/**
 * @brief Sets up the test fixture for each test.
 */
void LogLevelFilterItemWidgetTest::SetUp()
{
    m_widget = new LogLevelFilterItemWidget();
}

/**
 * @brief Tears down the test fixture after each test.
 */
void LogLevelFilterItemWidgetTest::TearDown()
{
    delete m_widget;
    m_widget = nullptr;
}

/**
 * @brief Helper to get the current checkbox object name.
 * @return The object name for the checkbox.
 */
auto LogLevelFilterItemWidgetTest::get_checkbox_object_name() const -> QString
{
    // Default name before set_log_level_name is called
    QString name = "checkBox";

    if (!m_widget->get_log_level_name().isEmpty())
    {
        name += m_widget->get_log_level_name();
    }

    return name;
}

/**
 * @brief Helper to get the checkbox widget using the current log level name.
 * @return Pointer to the QCheckBox, or nullptr.
 */
auto LogLevelFilterItemWidgetTest::get_checkbox() const -> QCheckBox*
{
    return m_widget->findChild<QCheckBox*>(get_checkbox_object_name());
}

/**
 * @brief Helper to get the count label widget.
 * @return Pointer to the QLabel, or nullptr.
 */
auto LogLevelFilterItemWidgetTest::get_count_label() const -> QLabel*
{
    return m_widget->findChild<QLabel*>("labelCount");
}

/**
 * @brief Tests construction and default state.
 */
TEST_F(LogLevelFilterItemWidgetTest, ConstructionAndDefaultState)
{
    EXPECT_NE(m_widget, nullptr);
    // Default checkbox name is "checkBox"
    auto* checkbox = m_widget->findChild<QCheckBox*>("checkBox");
    auto* count_label = get_count_label();
    EXPECT_NE(checkbox, nullptr);
    EXPECT_NE(count_label, nullptr);
    EXPECT_EQ(count_label->text(), "0");
    EXPECT_FALSE(checkbox->isChecked());
}

/**
 * @brief Tests setting and getting log level name.
 */
TEST_F(LogLevelFilterItemWidgetTest, SetAndGetLogLevelName)
{
    m_widget->set_log_level_name("Trace");
    EXPECT_EQ(m_widget->get_log_level_name(), "Trace");
    auto* checkbox = get_checkbox();
    ASSERT_NE(checkbox, nullptr);
    EXPECT_EQ(checkbox->text(), "Trace");
    EXPECT_EQ(checkbox->objectName(), "checkBoxTrace");
}

/**
 * @brief Tests setting count as text.
 */
TEST_F(LogLevelFilterItemWidgetTest, SetCountText)
{
    m_widget->set_count_text("42");
    auto* count_label = get_count_label();
    ASSERT_NE(count_label, nullptr);
    EXPECT_EQ(count_label->text(), "42");
}

/**
 * @brief Tests setting count as integer (abbreviated).
 */
TEST_F(LogLevelFilterItemWidgetTest, SetCountInt)
{
    m_widget->set_count(1200);
    auto* count_label = get_count_label();
    ASSERT_NE(count_label, nullptr);
    EXPECT_EQ(count_label->text(), "1.2K");
}

/**
 * @brief Tests setting and getting checked state.
 */
TEST_F(LogLevelFilterItemWidgetTest, SetAndGetCheckedState)
{
    m_widget->set_log_level_name("Debug");
    auto* checkbox = get_checkbox();
    ASSERT_NE(checkbox, nullptr);
    m_widget->set_checked(true);
    EXPECT_TRUE(m_widget->get_checked());
    m_widget->set_checked(false);
    EXPECT_FALSE(m_widget->get_checked());
}

/**
 * @brief Tests clear resets count and checkbox.
 */
TEST_F(LogLevelFilterItemWidgetTest, ClearResetsState)
{
    m_widget->set_log_level_name("Info");
    m_widget->set_count_text("99");
    m_widget->set_checked(true);
    m_widget->clear();
    auto* count_label = get_count_label();
    auto* checkbox = get_checkbox();
    ASSERT_NE(count_label, nullptr);
    ASSERT_NE(checkbox, nullptr);
    EXPECT_EQ(count_label->text(), "0");
    EXPECT_FALSE(checkbox->isChecked());
}

/**
 * @brief Tests toggled signal is emitted when checkbox is toggled.
 */
TEST_F(LogLevelFilterItemWidgetTest, ToggledSignalEmitted)
{
    m_widget->set_log_level_name("Warning");
    QSignalSpy spy(m_widget, SIGNAL(toggled(bool)));
    auto* checkbox = get_checkbox();
    ASSERT_NE(checkbox, nullptr);
    checkbox->setChecked(true);
    QTest::qWait(10);
    ASSERT_GE(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toBool(), true);
    checkbox->setChecked(false);
    QTest::qWait(10);
    ASSERT_GE(spy.count(), 1);
    EXPECT_EQ(spy.takeFirst().at(0).toBool(), false);
}
