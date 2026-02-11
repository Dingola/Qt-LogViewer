/**
 * @file WindowTitleBarTest.cpp
 * @brief Implements tests for the WindowTitleBar widget.
 */

#include "Qt-LogViewer/Views/Shared/WindowTitleBarTest.h"

#include <QAction>
#include <QApplication>
#include <QIcon>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMouseEvent>
#include <QPixmap>
#include <QPushButton>
#include <QSignalSpy>

void WindowTitleBarTest::SetUp()
{
    m_title_bar = new WindowTitleBar();
    m_title_bar->setMouseTracking(true);
    m_title_bar->resize(400, 60);
    m_title_bar->show();
    QApplication::processEvents();
}

void WindowTitleBarTest::TearDown()
{
    delete m_title_bar;
    m_title_bar = nullptr;
}

/**
 * @test Default properties are set and getters return initial values.
 */
TEST_F(WindowTitleBarTest, DefaultProperties)
{
    EXPECT_EQ(m_title_bar->get_window_button_icon_px(), 18);
    EXPECT_EQ(m_title_bar->get_window_button_color(), QColor("#000000"));

    EXPECT_NE(m_title_bar->get_minimize_button(), nullptr);
    EXPECT_NE(m_title_bar->get_maximize_button(), nullptr);
    EXPECT_NE(m_title_bar->get_close_button(), nullptr);

    EXPECT_EQ(m_title_bar->get_menubar(), nullptr);
    EXPECT_EQ(m_title_bar->get_custom_widget(), nullptr);

    auto* title = m_title_bar->findChild<QLabel*>(QStringLiteral("windowtitlebar_title"));
    ASSERT_NE(title, nullptr);
    EXPECT_FALSE(title->text().isEmpty());
}

/**
 * @test Setting icon and title updates internal widgets.
 */
TEST_F(WindowTitleBarTest, SetIconAndTitle)
{
    m_title_bar->set_title(QStringLiteral("MyTitle"));

    auto icon = QIcon(QPixmap(20, 20));
    m_title_bar->set_icon(icon);

    auto* title = m_title_bar->findChild<QLabel*>(QStringLiteral("windowtitlebar_title"));
    ASSERT_NE(title, nullptr);
    EXPECT_EQ(title->text(), QStringLiteral("MyTitle"));

    auto* icon_lbl = m_title_bar->findChild<QLabel*>(QStringLiteral("windowtitlebar_icon"));
    ASSERT_NE(icon_lbl, nullptr);
    EXPECT_TRUE(icon_lbl->isVisible());
}

/**
 * @test Changing window button color and size updates getters.
 */
TEST_F(WindowTitleBarTest, SetWindowButtonProperties)
{
    QColor new_color("#112233");
    m_title_bar->set_window_button_color(new_color);
    EXPECT_EQ(m_title_bar->get_window_button_color(), new_color);

    m_title_bar->set_window_button_icon_px(24);
    EXPECT_EQ(m_title_bar->get_window_button_icon_px(), 24);

    // Clamp behavior
    m_title_bar->set_window_button_icon_px(0);
    EXPECT_EQ(m_title_bar->get_window_button_icon_px(), 1);
}

/**
 * @test Emitting clicks from buttons forwards as signals.
 */
TEST_F(WindowTitleBarTest, ButtonSignals)
{
    QSignalSpy minimize_spy(m_title_bar, &WindowTitleBar::minimize_requested);
    QSignalSpy maximize_spy(m_title_bar, &WindowTitleBar::maximize_requested);
    QSignalSpy restore_spy(m_title_bar, &WindowTitleBar::restore_requested);
    QSignalSpy close_spy(m_title_bar, &WindowTitleBar::close_requested);

    ASSERT_TRUE(minimize_spy.isValid());
    ASSERT_TRUE(maximize_spy.isValid());
    ASSERT_TRUE(restore_spy.isValid());
    ASSERT_TRUE(close_spy.isValid());

    // Minimize
    QPushButton* min_btn = m_title_bar->get_minimize_button();
    ASSERT_NE(min_btn, nullptr);
    min_btn->click();
    QApplication::processEvents();
    EXPECT_GE(minimize_spy.count(), 1);

    // Maximize or restore depends on window state; ensure one of them fires via double-click
    QMouseEvent dbl_evt(QEvent::MouseButtonDblClick, QPoint(10, 10), Qt::LeftButton, Qt::LeftButton,
                        Qt::NoModifier);
    QApplication::sendEvent(m_title_bar, &dbl_evt);
    QApplication::processEvents();

    const int max_or_restore = maximize_spy.count() + restore_spy.count();
    EXPECT_GE(max_or_restore, 1);

    // Close
    QPushButton* close_btn = m_title_bar->get_close_button();
    ASSERT_NE(close_btn, nullptr);
    close_btn->click();
    QApplication::processEvents();
    EXPECT_GE(close_spy.count(), 1);
}

/**
 * @test Drag start signal is emitted on mouse press.
 */
TEST_F(WindowTitleBarTest, DragStartedSignal)
{
    QSignalSpy drag_spy(m_title_bar, &WindowTitleBar::drag_started);
    ASSERT_TRUE(drag_spy.isValid());

    QMouseEvent press_evt(QEvent::MouseButtonPress, QPoint(5, 5), Qt::LeftButton, Qt::LeftButton,
                          Qt::NoModifier);
    QApplication::sendEvent(m_title_bar, &press_evt);
    QApplication::processEvents();

    EXPECT_GE(drag_spy.count(), 1);

    QVariant last = drag_spy.takeLast().at(0);
    QPoint pt = last.toPoint();
    EXPECT_FALSE(pt.isNull());
}

/**
 * @test Menubar and custom widget placement and retrieval.
 */
TEST_F(WindowTitleBarTest, MenubarAndCustomWidget)
{
    // Menubar
    auto* menubar = new QMenuBar();
    auto* file_menu = new QMenu("File", menubar);
    QAction* act = file_menu->addAction("Quit");
    menubar->addMenu(file_menu);
    (void)act;

    m_title_bar->set_menubar(menubar);
    EXPECT_EQ(m_title_bar->get_menubar(), menubar);

    m_title_bar->set_menubar_row(WindowTitleBar::RowPosition::Bottom);
    QApplication::processEvents();
    EXPECT_EQ(m_title_bar->get_menubar(), menubar);

    // Custom widget
    auto* custom = new QWidget();
    m_title_bar->set_custom_widget(custom);
    EXPECT_EQ(m_title_bar->get_custom_widget(), custom);

    m_title_bar->set_custom_widget_row(WindowTitleBar::RowPosition::Bottom);
    QApplication::processEvents();
    EXPECT_EQ(m_title_bar->get_custom_widget(), custom);

    // Remove menubar and custom
    m_title_bar->set_menubar(nullptr);
    m_title_bar->set_custom_widget(nullptr);
    EXPECT_EQ(m_title_bar->get_menubar(), nullptr);
    EXPECT_EQ(m_title_bar->get_custom_widget(), nullptr);
}

/**
 * @test Event filter is installed on top-level window and reacts to window state change.
 */
TEST_F(WindowTitleBarTest, EventFilterInstalledAndUpdatesIcon)
{
    QWidget* top = m_title_bar->window();
    ASSERT_NE(top, nullptr);

    QPushButton* max_btn = m_title_bar->get_maximize_button();
    ASSERT_NE(max_btn, nullptr);

    const qint64 before_key = max_btn->icon().cacheKey();

    top->showMaximized();
    QApplication::processEvents();
    const qint64 maximized_key = max_btn->icon().cacheKey();

    top->showNormal();
    QApplication::processEvents();
    const qint64 normal_key = max_btn->icon().cacheKey();

    EXPECT_NE(before_key, maximized_key);
    EXPECT_NE(maximized_key, normal_key);
}

/**
 * @test Double-click emits maximize when normal, and restore when maximized.
 */
TEST_F(WindowTitleBarTest, DoubleClickMaximizeVsRestore)
{
    QSignalSpy maximize_spy(m_title_bar, &WindowTitleBar::maximize_requested);
    QSignalSpy restore_spy(m_title_bar, &WindowTitleBar::restore_requested);
    ASSERT_TRUE(maximize_spy.isValid());
    ASSERT_TRUE(restore_spy.isValid());

    QWidget* top = m_title_bar->window();
    ASSERT_NE(top, nullptr);

    top->showNormal();
    QApplication::processEvents();

    QMouseEvent dbl1(QEvent::MouseButtonDblClick, QPoint(10, 10), Qt::LeftButton, Qt::LeftButton,
                     Qt::NoModifier);
    QApplication::sendEvent(m_title_bar, &dbl1);
    QApplication::processEvents();
    EXPECT_GE(maximize_spy.count(), 1);

    top->showMaximized();
    QApplication::processEvents();

    QMouseEvent dbl2(QEvent::MouseButtonDblClick, QPoint(10, 10), Qt::LeftButton, Qt::LeftButton,
                     Qt::NoModifier);
    QApplication::sendEvent(m_title_bar, &dbl2);
    QApplication::processEvents();
    EXPECT_GE(restore_spy.count(), 1);
}
