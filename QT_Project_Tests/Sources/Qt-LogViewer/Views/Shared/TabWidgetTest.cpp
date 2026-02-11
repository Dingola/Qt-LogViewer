/**
 * @file TabWidgetTest.cpp
 * @brief Implements tests for the TabWidget (forwarded properties, signals, and default handlers).
 *
 * Test coverage:
 *  - Default forwarded properties and TabBar installation
 *  - Forwarding setters/getters to TabBar (incl. enable_default_context_menu)
 *  - Forwarded signals re-emitted by TabWidget and triggered via context menu interception
 *  - Default close handlers: single tab, others, left, right (triggered via TabBar/UI)
 *  - Disabled default context menu still notifies without actions and avoids blocking
 */

#include "Qt-LogViewer/Views/Shared/TabWidgetTest.h"

#include <QAction>
#include <QApplication>
#include <QMenu>
#include <QMouseEvent>
#include <QSignalSpy>
#include <QWidget>

#include "Qt-LogViewer/Views/Shared/TabBar.h"

void TabWidgetTest::SetUp()
{
    m_tab_widget = new TabWidget();
    m_tab_widget->resize(600, 300);
    m_tab_widget->show();

    // Populate with a few simple pages
    m_tab_widget->addTab(new QWidget(), QStringLiteral("One"));
    m_tab_widget->addTab(new QWidget(), QStringLiteral("Two"));
    m_tab_widget->addTab(new QWidget(), QStringLiteral("Three"));

    QApplication::processEvents();
}

void TabWidgetTest::TearDown()
{
    delete m_tab_widget;
    m_tab_widget = nullptr;
}

/**
 * @test Default forwarded properties are set and TabBar is installed.
 */
TEST_F(TabWidgetTest, DefaultForwardedProperties)
{
    ASSERT_NE(m_tab_widget, nullptr);
    auto* bar = qobject_cast<TabBar*>(m_tab_widget->tabBar());
    ASSERT_NE(bar, nullptr);

    EXPECT_EQ(m_tab_widget->get_close_icon_color(), QColor("#666666"));
    EXPECT_EQ(m_tab_widget->get_close_icon_color_hover(), QColor("#42a5f5"));
    EXPECT_EQ(m_tab_widget->get_close_icon_px(), 18);
    EXPECT_TRUE(m_tab_widget->get_enable_default_context_menu());

    EXPECT_EQ(m_tab_widget->count(), 3);
}

/**
 * @test Setting forwarded properties updates TabBar getters.
 */
TEST_F(TabWidgetTest, SetForwardedPropertiesUpdateTabBar)
{
    auto* bar = qobject_cast<TabBar*>(m_tab_widget->tabBar());
    ASSERT_NE(bar, nullptr);

    m_tab_widget->set_close_icon_color(QColor("#123456"));
    EXPECT_EQ(m_tab_widget->get_close_icon_color(), QColor("#123456"));
    EXPECT_EQ(bar->get_close_icon_color(), QColor("#123456"));

    m_tab_widget->set_close_icon_color_hover(QColor("#abcdef"));
    EXPECT_EQ(m_tab_widget->get_close_icon_color_hover(), QColor("#abcdef"));
    EXPECT_EQ(bar->get_close_icon_color_hover(), QColor("#abcdef"));

    m_tab_widget->set_close_icon_px(24);
    EXPECT_EQ(m_tab_widget->get_close_icon_px(), 24);
    EXPECT_EQ(bar->get_close_icon_px(), 24);

    m_tab_widget->set_enable_default_context_menu(false);
    EXPECT_FALSE(m_tab_widget->get_enable_default_context_menu());
    EXPECT_FALSE(bar->get_enable_default_context_menu());
}

/**
 * @test Forwarded signals from TabBar are re-emitted by TabWidget.
 *
 * Triggers default actions via intercepted context menu to avoid GUI blocking.
 */
TEST_F(TabWidgetTest, ForwardedSignalsFromTabBar)
{
    auto* bar = qobject_cast<TabBar*>(m_tab_widget->tabBar());
    ASSERT_NE(bar, nullptr);

    QSignalSpy about_spy(m_tab_widget, &TabWidget::about_to_show_context_menu);
    QSignalSpy close_spy(m_tab_widget, &TabWidget::close_tab_requested);
    QSignalSpy close_others_spy(m_tab_widget, &TabWidget::close_other_tabs_requested);
    QSignalSpy close_left_spy(m_tab_widget, &TabWidget::close_tabs_to_left_requested);
    QSignalSpy close_right_spy(m_tab_widget, &TabWidget::close_tabs_to_right_requested);

    ASSERT_TRUE(about_spy.isValid());
    ASSERT_TRUE(close_spy.isValid());
    ASSERT_TRUE(close_others_spy.isValid());
    ASSERT_TRUE(close_left_spy.isValid());
    ASSERT_TRUE(close_right_spy.isValid());

    m_tab_widget->set_enable_default_context_menu(true);

    QMetaObject::Connection conn = QObject::connect(
        m_tab_widget, &TabWidget::about_to_show_context_menu, m_tab_widget,
        [&](int idx, QMenu* menu) {
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

            menu->clear();  // avoid blocking

            EXPECT_EQ(idx, 1);
        },
        Qt::DirectConnection);

    const QPoint tab_pos = bar->tabRect(1).center();
    QContextMenuEvent ctx_evt(QContextMenuEvent::Mouse, tab_pos,
                              m_tab_widget->mapToGlobal(tab_pos));
    QApplication::sendEvent(bar, &ctx_evt);
    QApplication::processEvents();

    QObject::disconnect(conn);

    ASSERT_GE(about_spy.count(), 1);
    EXPECT_EQ(about_spy.takeLast().at(0).toInt(), 1);

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
 * @test Default close handler removes the requested tab and emits about_to_close_tab.
 *
 * Trigger via context menu action "Close" (non-blocking).
 */
TEST_F(TabWidgetTest, DefaultCloseHandlerRemovesTab)
{
    ASSERT_EQ(m_tab_widget->count(), 3);

    QSignalSpy pre_close_spy(m_tab_widget, &TabWidget::about_to_close_tab);
    ASSERT_TRUE(pre_close_spy.isValid());

    auto* bar = qobject_cast<TabBar*>(m_tab_widget->tabBar());
    ASSERT_NE(bar, nullptr);
    m_tab_widget->set_enable_default_context_menu(true);

    QMetaObject::Connection conn = QObject::connect(
        m_tab_widget, &TabWidget::about_to_show_context_menu, m_tab_widget,
        [&](int idx, QMenu* menu) {
            for (auto* a: menu->actions())
            {
                if (a->objectName() == QStringLiteral("tab_ctx_close"))
                {
                    a->trigger();
                }
            }
            menu->clear();
            EXPECT_EQ(idx, 1);
        },
        Qt::DirectConnection);

    const QPoint tab_pos = bar->tabRect(1).center();
    QContextMenuEvent ctx_evt(QContextMenuEvent::Mouse, tab_pos,
                              m_tab_widget->mapToGlobal(tab_pos));
    QApplication::sendEvent(bar, &ctx_evt);
    QApplication::processEvents();

    QObject::disconnect(conn);

    EXPECT_EQ(m_tab_widget->count(), 2);
    ASSERT_GE(pre_close_spy.count(), 1);

    const QList<QVariant> args = pre_close_spy.takeLast();
    ASSERT_EQ(args.size(), 2);
    EXPECT_EQ(args.at(0).toInt(), 1);
    EXPECT_TRUE(args.at(1).value<QWidget*>() != nullptr);
}

/**
 * @test Default handler closes other tabs, leaving only the requested one.
 *
 * Trigger via context menu action "Close Others" (non-blocking) and assert exact closed indices.
 */
TEST_F(TabWidgetTest, DefaultCloseOthersHandler)
{
    ASSERT_EQ(m_tab_widget->count(), 3);

    QSignalSpy pre_close_spy(m_tab_widget, &TabWidget::about_to_close_tab);
    ASSERT_TRUE(pre_close_spy.isValid());

    auto* bar = qobject_cast<TabBar*>(m_tab_widget->tabBar());
    ASSERT_NE(bar, nullptr);
    m_tab_widget->set_enable_default_context_menu(true);

    QMetaObject::Connection conn = QObject::connect(
        m_tab_widget, &TabWidget::about_to_show_context_menu, m_tab_widget,
        [&](int idx, QMenu* menu) {
            for (auto* a: menu->actions())
            {
                if (a->objectName() == QStringLiteral("tab_ctx_close_others"))
                {
                    a->trigger();
                }
            }
            menu->clear();
            EXPECT_EQ(idx, 1);
        },
        Qt::DirectConnection);

    const QPoint tab_pos = bar->tabRect(1).center();
    QContextMenuEvent ctx_evt(QContextMenuEvent::Mouse, tab_pos,
                              m_tab_widget->mapToGlobal(tab_pos));
    QApplication::sendEvent(bar, &ctx_evt);
    QApplication::processEvents();

    QObject::disconnect(conn);

    // Count check
    EXPECT_EQ(m_tab_widget->count(), 1);
    ASSERT_GE(pre_close_spy.count(), 2);

    // Verify exact indices closed: for "Close Others" on 1 -> expected {0, 2}
    QList<int> closed;
    for (const auto& emission: pre_close_spy)
    {
        if (!emission.isEmpty())
        {
            closed.push_back(emission.at(0).toInt());
        }
    }
    std::sort(closed.begin(), closed.end());
    const QList<int> expected{0, 2};
    EXPECT_EQ(closed, expected);
}

/**
 * @test Default handler closes tabs to the left of the given index.
 *
 * Trigger via context menu action "Close Tabs to Left" and assert exact closed indices.
 */
TEST_F(TabWidgetTest, DefaultCloseTabsToLeftHandler)
{
    ASSERT_EQ(m_tab_widget->count(), 3);

    QSignalSpy pre_close_spy(m_tab_widget, &TabWidget::about_to_close_tab);
    ASSERT_TRUE(pre_close_spy.isValid());

    auto* bar = qobject_cast<TabBar*>(m_tab_widget->tabBar());
    ASSERT_NE(bar, nullptr);
    m_tab_widget->set_enable_default_context_menu(true);

    QMetaObject::Connection conn = QObject::connect(
        m_tab_widget, &TabWidget::about_to_show_context_menu, m_tab_widget,
        [&](int idx, QMenu* menu) {
            for (auto* a: menu->actions())
            {
                if (a->objectName() == QStringLiteral("tab_ctx_close_left"))
                {
                    a->trigger();
                }
            }
            menu->clear();
            EXPECT_EQ(idx, 2);
        },
        Qt::DirectConnection);

    const QPoint tab_pos = bar->tabRect(2).center();
    QContextMenuEvent ctx_evt(QContextMenuEvent::Mouse, tab_pos,
                              m_tab_widget->mapToGlobal(tab_pos));
    QApplication::sendEvent(bar, &ctx_evt);
    QApplication::processEvents();

    QObject::disconnect(conn);

    // Count check
    EXPECT_EQ(m_tab_widget->count(), 1);
    ASSERT_GE(pre_close_spy.count(), 2);

    // Verify exact indices closed: for "Close Left" on 2 -> expected {0, 1}
    QList<int> closed;
    for (const auto& emission: pre_close_spy)
    {
        if (!emission.isEmpty())
        {
            closed.push_back(emission.at(0).toInt());
        }
    }
    std::sort(closed.begin(), closed.end());
    const QList<int> expected{0, 1};
    EXPECT_EQ(closed, expected);
}

/**
 * @test Default handler closes tabs to the right of the given index.
 *
 * Trigger via context menu action "Close Tabs to Right" and assert exact closed indices.
 */
TEST_F(TabWidgetTest, DefaultCloseTabsToRightHandler)
{
    ASSERT_EQ(m_tab_widget->count(), 3);

    QSignalSpy pre_close_spy(m_tab_widget, &TabWidget::about_to_close_tab);
    ASSERT_TRUE(pre_close_spy.isValid());

    auto* bar = qobject_cast<TabBar*>(m_tab_widget->tabBar());
    ASSERT_NE(bar, nullptr);
    m_tab_widget->set_enable_default_context_menu(true);

    QMetaObject::Connection conn = QObject::connect(
        m_tab_widget, &TabWidget::about_to_show_context_menu, m_tab_widget,
        [&](int idx, QMenu* menu) {
            for (auto* a: menu->actions())
            {
                if (a->objectName() == QStringLiteral("tab_ctx_close_right"))
                {
                    a->trigger();
                }
            }
            menu->clear();
            EXPECT_EQ(idx, 0);
        },
        Qt::DirectConnection);

    const QPoint tab_pos = bar->tabRect(0).center();
    QContextMenuEvent ctx_evt(QContextMenuEvent::Mouse, tab_pos,
                              m_tab_widget->mapToGlobal(tab_pos));
    QApplication::sendEvent(bar, &ctx_evt);
    QApplication::processEvents();

    QObject::disconnect(conn);

    // Count check
    EXPECT_EQ(m_tab_widget->count(), 1);
    ASSERT_GE(pre_close_spy.count(), 2);

    // Verify exact indices closed: for "Close Right" on 0 -> expected {1, 2}
    QList<int> closed;
    for (const auto& emission: pre_close_spy)
    {
        if (!emission.isEmpty())
        {
            closed.push_back(emission.at(0).toInt());
        }
    }
    std::sort(closed.begin(), closed.end());
    const QList<int> expected{1, 2};
    EXPECT_EQ(closed, expected);
}

/**
 * @test Disabling default context menu still notifies via about_to_show_context_menu and keeps menu
 * empty.
 */
TEST_F(TabWidgetTest, ContextMenuDisabledStillNotifies)
{
    auto* bar = qobject_cast<TabBar*>(m_tab_widget->tabBar());
    ASSERT_NE(bar, nullptr);

    m_tab_widget->set_enable_default_context_menu(false);
    EXPECT_FALSE(m_tab_widget->get_enable_default_context_menu());

    QSignalSpy about_spy(m_tab_widget, &TabWidget::about_to_show_context_menu);
    ASSERT_TRUE(about_spy.isValid());

    QMetaObject::Connection conn = QObject::connect(
        m_tab_widget, &TabWidget::about_to_show_context_menu, m_tab_widget,
        [&](int idx, QMenu* menu) {
            EXPECT_EQ(idx, 0);
            EXPECT_TRUE(menu->actions().isEmpty());
            menu->clear();
        },
        Qt::DirectConnection);

    const QPoint tab_pos = bar->tabRect(0).center();
    QContextMenuEvent ctx_evt(QContextMenuEvent::Mouse, tab_pos,
                              m_tab_widget->mapToGlobal(tab_pos));
    QApplication::sendEvent(bar, &ctx_evt);
    QApplication::processEvents();

    QObject::disconnect(conn);

    ASSERT_GE(about_spy.count(), 1);
    EXPECT_EQ(about_spy.takeLast().at(0).toInt(), 0);
}
