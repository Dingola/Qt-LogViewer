/**
 * @file TabBarTest.cpp
 * @brief Implements tests for the TabBar widget (close icon properties, buttons, and context menu).
 */

#include "Qt-LogViewer/Views/Shared/TabBarTest.h"

#include <QAbstractButton>
#include <QAction>
#include <QApplication>
#include <QContextMenuEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QSignalSpy>
#include <QTabBar>

void TabBarTest::SetUp()
{
    m_tab_bar = new TabBar();
    m_tab_bar->setTabsClosable(true);
    m_tab_bar->resize(400, 40);
    m_tab_bar->show();

    // Create a few tabs to ensure buttons exist
    m_tab_bar->addTab(QStringLiteral("One"));
    m_tab_bar->addTab(QStringLiteral("Two"));
    m_tab_bar->addTab(QStringLiteral("Three"));

    QApplication::processEvents();
}

void TabBarTest::TearDown()
{
    delete m_tab_bar;
    m_tab_bar = nullptr;
}

/**
 * @test Default properties are set and getters return initial values.
 */
TEST_F(TabBarTest, DefaultProperties)
{
    EXPECT_EQ(m_tab_bar->get_close_icon_color(), QColor("#666666"));
    EXPECT_EQ(m_tab_bar->get_close_icon_color_hover(), QColor("#42a5f5"));
    EXPECT_EQ(m_tab_bar->get_close_icon_px(), 18);
    EXPECT_TRUE(m_tab_bar->get_enable_default_context_menu());

    // Each tab should have a close tool button on the RightSide
    const int count = m_tab_bar->count();
    ASSERT_GT(count, 0);
    for (int i = 0; i < count; ++i)
    {
        QWidget* btn = m_tab_bar->tabButton(i, QTabBar::RightSide);
        EXPECT_NE(btn, nullptr);
    }
}

/**
 * @test Changing close icon properties updates internal state and re-applies buttons.
 */
TEST_F(TabBarTest, SetCloseIconProperties)
{
    m_tab_bar->set_close_icon_color(QColor("#123456"));
    EXPECT_EQ(m_tab_bar->get_close_icon_color(), QColor("#123456"));

    m_tab_bar->set_close_icon_color_hover(QColor("#abcdef"));
    EXPECT_EQ(m_tab_bar->get_close_icon_color_hover(), QColor("#abcdef"));

    m_tab_bar->set_close_icon_px(24);
    EXPECT_EQ(m_tab_bar->get_close_icon_px(), 24);

    // Clamp behavior
    m_tab_bar->set_close_icon_px(0);
    EXPECT_EQ(m_tab_bar->get_close_icon_px(), 1);

    // Buttons should still exist after updates
    const int count = m_tab_bar->count();
    for (int i = 0; i < count; ++i)
    {
        QWidget* btn = m_tab_bar->tabButton(i, QTabBar::RightSide);
        EXPECT_NE(btn, nullptr);
    }
}

/**
 * @test tabInserted ensures a close button exists for the new tab; showEvent refreshes icons.
 */
TEST_F(TabBarTest, TabInsertedAndShowEventRefresh)
{
    const int initial = m_tab_bar->count();
    m_tab_bar->addTab(QStringLiteral("Four"));
    QApplication::processEvents();

    EXPECT_EQ(m_tab_bar->count(), initial + 1);

    QWidget* btn = m_tab_bar->tabButton(initial, QTabBar::RightSide);
    EXPECT_NE(btn, nullptr);

    // Trigger showEvent to force re-apply
    m_tab_bar->hide();
    m_tab_bar->show();
    QApplication::processEvents();

    QWidget* btn_after = m_tab_bar->tabButton(initial, QTabBar::RightSide);
    EXPECT_NE(btn_after, nullptr);
}

/**
 * @test Hover and press events on close buttons should change icon from normal, and leave should
 * change away from hover.
 *
 * Do not assume equal cacheKeys for hover vs press; QIcon instances may differ.
 */
TEST_F(TabBarTest, EventFilterHoverAndPress)
{
    ASSERT_GT(m_tab_bar->count(), 0);
    auto* btn = qobject_cast<QAbstractButton*>(m_tab_bar->tabButton(0, QTabBar::RightSide));
    ASSERT_NE(btn, nullptr);

    const qint64 normal_key_before = btn->icon().cacheKey();

    // Simulate hover enter -> icon should change from the normal state
    QEvent enter_evt(QEvent::Enter);
    QApplication::sendEvent(btn, &enter_evt);
    QApplication::processEvents();
    const qint64 hover_key = btn->icon().cacheKey();
    EXPECT_NE(hover_key, normal_key_before);

    // Simulate mouse press -> icon should remain not-normal (may be a new QIcon instance)
    QMouseEvent press_evt(QEvent::MouseButtonPress, QPoint(1, 1), Qt::LeftButton, Qt::LeftButton,
                          Qt::NoModifier);
    QApplication::sendEvent(btn, &press_evt);
    QApplication::processEvents();
    const qint64 press_key = btn->icon().cacheKey();
    EXPECT_NE(press_key, normal_key_before);

    // Simulate leave -> icon should change away from hover/press state
    QEvent leave_evt(QEvent::Leave);
    QApplication::sendEvent(btn, &leave_evt);
    QApplication::processEvents();
    const qint64 normal_key_after = btn->icon().cacheKey();
    EXPECT_NE(normal_key_after, hover_key);
}

/**
 * @test Clicking the close button emits close_tab_requested for the correct index.
 */
TEST_F(TabBarTest, CloseButtonEmitsSignalForIndex)
{
    ASSERT_GE(m_tab_bar->count(), 3);

    QSignalSpy close_spy(m_tab_bar, &TabBar::close_tab_requested);
    ASSERT_TRUE(close_spy.isValid());

    // Click close button of second tab (index 1)
    auto* btn = qobject_cast<QAbstractButton*>(m_tab_bar->tabButton(1, QTabBar::RightSide));
    ASSERT_NE(btn, nullptr);
    btn->click();
    QApplication::processEvents();

    ASSERT_GE(close_spy.count(), 1);
    const QVariant last = close_spy.takeLast().at(0);
    const int idx = last.toInt();
    EXPECT_EQ(idx, 1);
}

/**
 * @test Default context menu actions emit signals and do not block (menu cleared in signal
 * handler).
 */
TEST_F(TabBarTest, ContextMenuDefaultActionsAndSignals)
{
    ASSERT_GE(m_tab_bar->count(), 3);

    // Ensure default context menu enabled
    m_tab_bar->set_enable_default_context_menu(true);
    EXPECT_TRUE(m_tab_bar->get_enable_default_context_menu());

    QSignalSpy about_spy(m_tab_bar, &TabBar::about_to_show_context_menu);
    QSignalSpy close_spy(m_tab_bar, &TabBar::close_tab_requested);
    QSignalSpy close_others_spy(m_tab_bar, &TabBar::close_other_tabs_requested);
    QSignalSpy close_left_spy(m_tab_bar, &TabBar::close_tabs_to_left_requested);
    QSignalSpy close_right_spy(m_tab_bar, &TabBar::close_tabs_to_right_requested);

    ASSERT_TRUE(about_spy.isValid());
    ASSERT_TRUE(close_spy.isValid());
    ASSERT_TRUE(close_others_spy.isValid());
    ASSERT_TRUE(close_left_spy.isValid());
    ASSERT_TRUE(close_right_spy.isValid());

    // Intercept the signal to access the live QMenu, trigger actions, then clear to avoid exec()
    QMetaObject::Connection conn = QObject::connect(
        m_tab_bar, &TabBar::about_to_show_context_menu, m_tab_bar,
        [&](int idx, QMenu* menu) {
            // Expect default actions present
            QAction* act_close = nullptr;
            QAction* act_close_others = nullptr;
            QAction* act_close_left = nullptr;
            QAction* act_close_right = nullptr;
            for (auto* a: menu->actions())
            {
                const QString name = a->objectName();
                if (name == QStringLiteral("tab_ctx_close"))
                    act_close = a;
                else if (name == QStringLiteral("tab_ctx_close_others"))
                    act_close_others = a;
                else if (name == QStringLiteral("tab_ctx_close_left"))
                    act_close_left = a;
                else if (name == QStringLiteral("tab_ctx_close_right"))
                    act_close_right = a;
            }

            if (act_close != nullptr) act_close->trigger();
            if (act_close_others != nullptr) act_close_others->trigger();
            if (act_close_left != nullptr) act_close_left->trigger();
            if (act_close_right != nullptr) act_close_right->trigger();

            // Prevent blocking context menu by clearing items
            menu->clear();

            EXPECT_EQ(idx, 1);
        },
        Qt::DirectConnection);

    // Open context menu on middle tab (index 1)
    const QPoint tab_pos = m_tab_bar->tabRect(1).center();
    QContextMenuEvent ctx_evt(QContextMenuEvent::Mouse, tab_pos, m_tab_bar->mapToGlobal(tab_pos));
    QApplication::sendEvent(m_tab_bar, &ctx_evt);
    QApplication::processEvents();

    QObject::disconnect(conn);

    // Signal should be emitted with index
    ASSERT_GE(about_spy.count(), 1);
    EXPECT_EQ(about_spy.takeLast().at(0).toInt(), 1);

    // Actions must have emitted their signals with index 1
    ASSERT_GE(close_spy.count(), 1);
    EXPECT_EQ(close_spy.takeLast().at(0).toInt(), 1);

    ASSERT_GE(close_others_spy.count(), 1);
    EXPECT_EQ(close_others_spy.takeLast().at(0).toInt(), 1);

    ASSERT_GE(close_left_spy.count(), 1);
    EXPECT_EQ(close_left_spy.takeLast().at(0).toInt(), 1);

    ASSERT_GE(close_right_spy.count(), 1);
    EXPECT_EQ(close_right_spy.takeLast().at(0).toInt(), 1);
}

/**
 * @test Disabling default context menu should still emit about_to_show_context_menu but not exec.
 */
TEST_F(TabBarTest, ContextMenuDisabledStillNotifies)
{
    m_tab_bar->set_enable_default_context_menu(false);
    EXPECT_FALSE(m_tab_bar->get_enable_default_context_menu());

    QSignalSpy about_spy(m_tab_bar, &TabBar::about_to_show_context_menu);
    ASSERT_TRUE(about_spy.isValid());

    // Also ensure we clear the (empty) menu to avoid exec
    QMetaObject::Connection conn = QObject::connect(
        m_tab_bar, &TabBar::about_to_show_context_menu, m_tab_bar,
        [&](int idx, QMenu* menu) {
            EXPECT_EQ(idx, 0);
            EXPECT_TRUE(menu->actions().isEmpty());
            menu->clear();
        },
        Qt::DirectConnection);

    const QPoint tab_pos = m_tab_bar->tabRect(0).center();
    QContextMenuEvent ctx_evt(QContextMenuEvent::Mouse, tab_pos, m_tab_bar->mapToGlobal(tab_pos));
    QApplication::sendEvent(m_tab_bar, &ctx_evt);
    QApplication::processEvents();

    QObject::disconnect(conn);

    ASSERT_GE(about_spy.count(), 1);
    EXPECT_EQ(about_spy.takeLast().at(0).toInt(), 0);
}
