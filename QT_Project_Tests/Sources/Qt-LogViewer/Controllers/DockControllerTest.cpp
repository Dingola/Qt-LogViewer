#include "Qt-LogViewer/Controllers/DockControllerTest.h"

#include <QApplication>
#include <QDockWidget>
#include <QEventLoop>
#include <QMainWindow>
#include <QTimer>
#include <QWidget>

#include "Qt-LogViewer/Controllers/DockController.h"

/**
 * @brief Constructs the test fixture.
 */
DockControllerTest::DockControllerTest() = default;

/**
 * @brief Destroys the test fixture.
 */
DockControllerTest::~DockControllerTest() = default;

/**
 * @brief Processes pending Qt events until dock layouts have settled.
 */
auto DockControllerTest::process_layout_events() -> void
{
    QApplication::sendPostedEvents();
    QApplication::processEvents();

    QEventLoop event_loop;
    QTimer::singleShot(0, &event_loop,
                       [&event_loop] { QTimer::singleShot(0, &event_loop, &QEventLoop::quit); });
    event_loop.exec();

    QApplication::sendPostedEvents();
    QApplication::processEvents();
}

/**
 * @brief Repeating suspension must hide docks made visible by a later layout restoration.
 */
TEST_F(DockControllerTest, ReappliesSuspensionAfterLayoutRestore)
{
    QMainWindow main_window;
    main_window.setCentralWidget(new QWidget(&main_window));

    auto* left_dock = new QDockWidget(QStringLiteral("Left"), &main_window);
    left_dock->setObjectName(QStringLiteral("leftDock"));
    left_dock->setWidget(new QWidget(left_dock));
    main_window.addDockWidget(Qt::LeftDockWidgetArea, left_dock);
    main_window.show();
    process_layout_events();

    const QByteArray visible_layout = main_window.saveState();
    DockController controller(&main_window);
    controller.register_dock(left_dock);
    controller.set_docks_suspended(true);
    EXPECT_FALSE(left_dock->isVisible());
    EXPECT_FALSE(left_dock->toggleViewAction()->isEnabled());

    main_window.restoreState(visible_layout);
    process_layout_events();
    ASSERT_TRUE(left_dock->isVisible());

    controller.set_docks_suspended(true);
    EXPECT_FALSE(left_dock->isVisible());
    EXPECT_FALSE(left_dock->toggleViewAction()->isEnabled());

    controller.set_docks_suspended(false);
    process_layout_events();
    EXPECT_TRUE(left_dock->isVisible());
    EXPECT_TRUE(left_dock->toggleViewAction()->isEnabled());
}

/**
 * @brief Repeated diagonal resizing of a main window with mixed dock areas must not change the dock
 * extents.
 */
TEST_F(DockControllerTest, MixedDockAreasKeepExtentsAcrossRepeatedDiagonalResizes)
{
    QMainWindow window;
    auto* central = new QWidget(&window);
    central->setMinimumSize(250, 180);
    window.setCentralWidget(central);
    auto* left = new QDockWidget(QStringLiteral("Left"), &window);
    auto* bottom = new QDockWidget(QStringLiteral("Bottom"), &window);
    left->setWidget(new QWidget(left));
    bottom->setWidget(new QWidget(bottom));
    window.addDockWidget(Qt::LeftDockWidgetArea, left);
    window.addDockWidget(Qt::BottomDockWidgetArea, bottom);
    window.resize(1000, 800);
    window.show();
    process_layout_events();
    window.resizeDocks({left}, {300}, Qt::Horizontal);
    window.resizeDocks({bottom}, {220}, Qt::Vertical);
    process_layout_events();
    DockController controller(&window);
    controller.register_dock(left);
    controller.register_dock(bottom);
    for (int cycle = 0; cycle < 4; ++cycle)
    {
        for (int step = 0; step <= 60; ++step)
        {
            window.resize(1000 - step * 10, 800 - step * 8);
            controller.handle_main_window_resize(
                QSize(1000 - (step - 1) * 10, 800 - (step - 1) * 8), window.size());
            process_layout_events();
        }
        for (int step = 60; step >= 0; --step)
        {
            window.resize(1000 - step * 10, 800 - step * 8);
            controller.handle_main_window_resize(
                QSize(1000 - (step + 1) * 10, 800 - (step + 1) * 8), window.size());
            const int left_width = left->width();
            const int bottom_height = bottom->height();
            process_layout_events();
            EXPECT_EQ(left->width(), left_width);
            EXPECT_EQ(bottom->height(), bottom_height);
        }
        EXPECT_NEAR(left->width(), 300, 2);
        EXPECT_NEAR(bottom->height(), 220, 2);
    }
    window.resizeDocks({left}, {250}, Qt::Horizontal);
    process_layout_events();
    const int updated_preferred_width = left->width();
    controller.capture_current_sizes();
    window.resize(400, 320);
    controller.handle_main_window_resize(QSize(1000, 800), window.size());
    process_layout_events();
    window.resize(1000, 800);
    controller.handle_main_window_resize(QSize(400, 320), window.size());
    process_layout_events();
    EXPECT_NEAR(left->width(), updated_preferred_width, 2);
}

/**
 * @brief A temporary main-window width reduction must not replace the user's dock width.
 */
TEST_F(DockControllerTest, RestoresPreferredDockWidthAfterWindowGrowsAgain)
{
    QMainWindow main_window;
    auto* central_widget = new QWidget(&main_window);
    central_widget->setMinimumWidth(250);
    main_window.setCentralWidget(central_widget);

    auto* left_dock = new QDockWidget(QStringLiteral("Left"), &main_window);
    left_dock->setWidget(new QWidget(left_dock));
    main_window.addDockWidget(Qt::LeftDockWidgetArea, left_dock);
    main_window.resize(900, 600);
    main_window.show();
    process_layout_events();

    main_window.resizeDocks({left_dock}, {300}, Qt::Horizontal);
    process_layout_events();
    DockController controller(&main_window);
    controller.register_dock(left_dock);
    process_layout_events();
    const int preferred_width = left_dock->width();

    for (int cycle = 0; cycle < 5; ++cycle)
    {
        for (int width = 900; width >= 380; width -= 10)
        {
            main_window.resize(width, 600);
            controller.handle_main_window_resize(QSize(width + 10, 600), main_window.size());
            process_layout_events();
        }
        for (int width = 380; width <= 900; width += 10)
        {
            main_window.resize(width, 600);
            controller.handle_main_window_resize(QSize(width - 10, 600), main_window.size());
            const int synchronous_width = left_dock->width();
            process_layout_events();
            EXPECT_EQ(left_dock->width(), synchronous_width);
        }
        EXPECT_NEAR(left_dock->width(), preferred_width, 2);
    }

    EXPECT_NEAR(left_dock->width(), preferred_width, 2);
}

/**
 * @brief A temporary main-window height reduction must not replace the user's dock height.
 */
TEST_F(DockControllerTest, RestoresPreferredDockHeightAfterWindowGrowsAgain)
{
    QMainWindow main_window;
    auto* central_widget = new QWidget(&main_window);
    central_widget->setMinimumHeight(180);
    main_window.setCentralWidget(central_widget);

    auto* bottom_dock = new QDockWidget(QStringLiteral("Bottom"), &main_window);
    bottom_dock->setWidget(new QWidget(bottom_dock));
    main_window.addDockWidget(Qt::BottomDockWidgetArea, bottom_dock);
    main_window.resize(900, 700);
    main_window.show();
    process_layout_events();

    main_window.resizeDocks({bottom_dock}, {220}, Qt::Vertical);
    process_layout_events();
    DockController controller(&main_window);
    controller.register_dock(bottom_dock);
    process_layout_events();
    const int preferred_height = bottom_dock->height();

    main_window.resize(900, 300);
    controller.handle_main_window_resize(QSize(900, 700), main_window.size());
    process_layout_events();
    main_window.resize(900, 700);
    controller.handle_main_window_resize(QSize(900, 300), main_window.size());
    process_layout_events();

    EXPECT_NEAR(bottom_dock->height(), preferred_height, 2);
}
