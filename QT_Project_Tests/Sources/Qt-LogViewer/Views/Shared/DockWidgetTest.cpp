#include "Qt-LogViewer/Views/Shared/DockWidgetTest.h"

#include <QApplication>
#include <QEvent>
#include <QMainWindow>
#include <QTest>
#include <QToolButton>

/**
 * @brief Constructs the test fixture.
 */
DockWidgetTest::DockWidgetTest(): m_dock(nullptr) {}

/**
 * @brief Destroys the test fixture.
 */
DockWidgetTest::~DockWidgetTest() = default;

/**
 * @brief Set up before each test: create and show a dock widget.
 */
void DockWidgetTest::SetUp()
{
    m_dock = new DockWidget();
    m_dock->setWindowTitle(QStringLiteral("Dock Under Test"));
    m_dock->show();
    QApplication::processEvents();
}

/**
 * @brief Tear down after each test.
 */
void DockWidgetTest::TearDown()
{
    delete m_dock;
    m_dock = nullptr;
    QApplication::processEvents();
}

/**
 * @brief Creates and installs the custom title bar on the given dock widget.
 */
auto DockWidgetTest::create_and_set_title_bar(QDockWidget* dock) -> QWidget*
{
    QWidget* title_bar = DockWidget::create_dock_title_bar(dock);
    if (dock != nullptr)
    {
        dock->setTitleBarWidget(title_bar);
    }
    QWidget* result = title_bar;
    return result;
}

/**
 * @brief Finds the close button within a title-bar widget.
 */
auto DockWidgetTest::find_close_button(QWidget* title_bar) const -> QToolButton*
{
    QToolButton* button = nullptr;
    if (title_bar != nullptr)
    {
        auto* found =
            title_bar->findChild<QToolButton*>(QStringLiteral("dock_title_bar_close_button"));
        button = found;
    }
    QToolButton* result = button;
    return result;
}

/**
 * @brief Defaults are as documented: colors and size.
 */
TEST_F(DockWidgetTest, DefaultsAsDocumented)
{
    ASSERT_NE(m_dock, nullptr);

    EXPECT_EQ(m_dock->get_close_icon_color(), QColor("#666666"));
    EXPECT_EQ(m_dock->get_close_icon_color_hover(), QColor("#42a5f5"));
    EXPECT_EQ(m_dock->get_close_icon_px(), 18);
}

/**
 * @brief Static title-bar creation wires close button and DockWidget emits `closed` on close.
 */
TEST_F(DockWidgetTest, CreateTitleBarAndCloseEmitsClosed)
{
    ASSERT_NE(m_dock, nullptr);

    QWidget* title_bar = create_and_set_title_bar(m_dock);
    ASSERT_NE(title_bar, nullptr);

    QToolButton* close_btn = find_close_button(title_bar);
    ASSERT_NE(close_btn, nullptr);

    QSignalSpy spy_closed(m_dock, &DockWidget::closed);

    // Click close -> emits `closed` and hides the dock
    close_btn->click();
    QApplication::processEvents();

    EXPECT_EQ(spy_closed.count(), 1);
    EXPECT_TRUE(m_dock->isHidden());
}

/**
 * @brief Setters update getters and reflect on the close button size (with clamping).
 */
TEST_F(DockWidgetTest, SettersUpdateValuesAndButtonSize)
{
    ASSERT_NE(m_dock, nullptr);

    QWidget* title_bar = create_and_set_title_bar(m_dock);
    ASSERT_NE(title_bar, nullptr);
    QToolButton* close_btn = find_close_button(title_bar);
    ASSERT_NE(close_btn, nullptr);

    // Change icon size
    m_dock->set_close_icon_px(24);
    EXPECT_EQ(m_dock->get_close_icon_px(), 24);
    EXPECT_EQ(close_btn->iconSize(), QSize(24, 24));

    // Clamp to 1
    m_dock->set_close_icon_px(0);
    EXPECT_EQ(m_dock->get_close_icon_px(), 1);
    EXPECT_EQ(close_btn->iconSize(), QSize(1, 1));

    // Change colors (no crash; getters reflect values)
    const QColor base("#111111");
    const QColor hover("#222222");
    m_dock->set_close_icon_color(base);
    m_dock->set_close_icon_color_hover(hover);

    EXPECT_EQ(m_dock->get_close_icon_color(), base);
    EXPECT_EQ(m_dock->get_close_icon_color_hover(), hover);
}

/**
 * @brief Show/hover/press/release path updates icons via event filter; ensure no crash.
 *
 * Use QTest mouse helpers to generate valid QMouseEvent sequences.
 */
TEST_F(DockWidgetTest, HoverPressReleaseEventsDoNotCrash)
{
    ASSERT_NE(m_dock, nullptr);

    QWidget* title_bar = create_and_set_title_bar(m_dock);
    ASSERT_NE(title_bar, nullptr);
    QToolButton* close_btn = find_close_button(title_bar);
    ASSERT_NE(close_btn, nullptr);

    // Ensure dock/button are visible to receive events
    m_dock->show();
    close_btn->show();
    QApplication::processEvents();

    // Move mouse over the button (hover)
    QTest::mouseMove(close_btn, QPoint(2, 2));

    // Press and release left mouse button on the button
    QTest::mousePress(close_btn, Qt::LeftButton, Qt::NoModifier, QPoint(2, 2));
    QTest::mouseRelease(close_btn, Qt::LeftButton, Qt::NoModifier, QPoint(2, 2));

    // Move mouse away to trigger leave
    QPoint outside(close_btn->width() + 5, close_btn->height() + 5);
    QTest::mouseMove(close_btn, outside);

    SUCCEED();
}

/**
 * @brief Fallback path: using the static creator for a plain QDockWidget also yields a working
 * close button.
 */
TEST_F(DockWidgetTest, StaticCreateWorksForPlainQDockWidget)
{
    QDockWidget plain;
    plain.setWindowTitle(QStringLiteral("Plain Dock"));
    plain.show();

    QWidget* title_bar = DockWidget::create_dock_title_bar(&plain);
    ASSERT_NE(title_bar, nullptr);
    plain.setTitleBarWidget(title_bar);

    auto* close_btn =
        title_bar->findChild<QToolButton*>(QStringLiteral("dock_title_bar_close_button"));
    ASSERT_NE(close_btn, nullptr);

    // Clicking should close/hide the plain dock (no `closed` signal on plain dock)
    close_btn->click();
    QApplication::processEvents();

    EXPECT_TRUE(plain.isHidden());
}
