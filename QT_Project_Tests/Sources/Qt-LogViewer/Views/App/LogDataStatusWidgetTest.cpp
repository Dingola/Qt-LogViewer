#include "Qt-LogViewer/Views/App/LogDataStatusWidgetTest.h"

#include <QLabel>
#include <QLocale>

#include "Qt-LogViewer/Views/App/LogDataStatusWidget.h"

/**
 * @file LogDataStatusWidgetTest.cpp
 * @brief Implements tests for the compact log data status widget.
 */

/** @brief Creates a fresh data status widget. */
void LogDataStatusWidgetTest::SetUp()
{
    m_widget = new LogDataStatusWidget();
}

/** @brief Destroys the data status widget. */
void LogDataStatusWidgetTest::TearDown()
{
    delete m_widget;
    m_widget = nullptr;
}

/** @test Verifies that the widget starts hidden without an active log view. */
TEST_F(LogDataStatusWidgetTest, StartsHiddenWithoutActiveView)
{
    EXPECT_TRUE(m_widget->isHidden());
}

/** @test Verifies formatted active-view values and the current-cache state. */
TEST_F(LogDataStatusWidgetTest, PresentsCompactActiveViewStatus)
{
    m_widget->set_status(12431, 83902, 2, 3, LogCacheStatus::Current);

    const QLabel* entries = m_widget->findChild<QLabel*>(QStringLiteral("labelEntryStatus"));
    const QLabel* sources = m_widget->findChild<QLabel*>(QStringLiteral("labelSourceStatus"));
    const QLabel* filters = m_widget->findChild<QLabel*>(QStringLiteral("labelFilterStatus"));
    const QLabel* cache = m_widget->findChild<QLabel*>(QStringLiteral("labelCacheStatus"));
    ASSERT_NE(entries, nullptr);
    ASSERT_NE(sources, nullptr);
    ASSERT_NE(filters, nullptr);
    ASSERT_NE(cache, nullptr);

    const QLocale locale;
    EXPECT_TRUE(entries->text().contains(locale.toString(12431)));
    EXPECT_TRUE(entries->text().contains(locale.toString(83902)));
    EXPECT_TRUE(sources->text().contains(QStringLiteral("2")));
    EXPECT_TRUE(filters->text().contains(QStringLiteral("3")));
    EXPECT_EQ(cache->property("cacheState").toString(), QStringLiteral("current"));
    EXPECT_TRUE(cache->text().contains(QStringLiteral("Index ready")));
    EXPECT_FALSE(m_widget->isHidden());
}

/** @test Verifies million-scale counts are abbreviated while tooltips retain exact values. */
TEST_F(LogDataStatusWidgetTest, AbbreviatesMillionScaleCountsWithExactTooltip)
{
    m_widget->set_status(1243892, 8731204, 2, 1, LogCacheStatus::Current);

    const QLabel* entries = m_widget->findChild<QLabel*>(QStringLiteral("labelEntryStatus"));
    ASSERT_NE(entries, nullptr);

    const QLocale locale;
    EXPECT_EQ(entries->text(), QStringLiteral("1.2M visible / 8.7M total"));
    EXPECT_TRUE(entries->toolTip().contains(locale.toString(1243892)));
    EXPECT_TRUE(entries->toolTip().contains(locale.toString(8731204)));
}

/** @test Verifies updating cache presentation and subsequent inactive-view clearing. */
TEST_F(LogDataStatusWidgetTest, PresentsUpdatingCacheAndCanBeCleared)
{
    m_widget->set_status(0, 0, 1, 0, LogCacheStatus::Updating);

    const QLabel* cache = m_widget->findChild<QLabel*>(QStringLiteral("labelCacheStatus"));
    ASSERT_NE(cache, nullptr);
    EXPECT_EQ(cache->property("cacheState").toString(), QStringLiteral("updating"));
    EXPECT_TRUE(cache->text().contains(QStringLiteral("Index is being built")));

    m_widget->clear_status();
    EXPECT_TRUE(m_widget->isHidden());
    EXPECT_TRUE(cache->text().isEmpty());
}

/** @test Verifies unavailable internal cache state does not expose a technical status. */
TEST_F(LogDataStatusWidgetTest, HidesUnavailableIndexStatus)
{
    m_widget->set_status(42, 42, 1, 0, LogCacheStatus::Unavailable);

    const QLabel* separator = m_widget->findChild<QLabel*>(QStringLiteral("separatorCacheStatus"));
    const QLabel* cache = m_widget->findChild<QLabel*>(QStringLiteral("labelCacheStatus"));
    ASSERT_NE(separator, nullptr);
    ASSERT_NE(cache, nullptr);
    EXPECT_TRUE(separator->isHidden());
    EXPECT_TRUE(cache->isHidden());
    EXPECT_TRUE(cache->text().isEmpty());
}
