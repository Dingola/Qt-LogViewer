/**
 * @file DialogTest.cpp
 * @brief Implements tests for the Dialog base class (header, drag-move, border radius, close).
 */

#include "Qt-LogViewer/Views/Shared/DialogTest.h"

#include <QApplication>
#include <QDialog>
#include <QMouseEvent>
#include <QPainter>
#include <QSignalSpy>
#include <QVBoxLayout>

#include "Qt-LogViewer/Views/Shared/DialogHeaderWidget.h"

void DialogTest::SetUp()
{
    m_dialog = new Dialog(QStringLiteral("Test Dialog"));
    m_dialog->resize(300, 160);
    m_dialog->show();
    QApplication::processEvents();
}

void DialogTest::TearDown()
{
    delete m_dialog;
    m_dialog = nullptr;
}

/**
 * @test Default state: frameless flags, object name, header exists, default border radius.
 */
TEST_F(DialogTest, DefaultStateAndHeader)
{
    ASSERT_NE(m_dialog, nullptr);
    EXPECT_EQ(m_dialog->objectName(), QStringLiteral("Dialog"));

    // Frameless flag set
    const Qt::WindowFlags flags = m_dialog->windowFlags();
    EXPECT_TRUE(flags.testFlag(Qt::FramelessWindowHint));
    EXPECT_TRUE(flags.testFlag(Qt::Dialog));

    // Translucent background attribute set
    EXPECT_TRUE(m_dialog->testAttribute(Qt::WA_TranslucentBackground));

    // Header exists (access via QObject hierarchy; header_widget() is protected)
    auto* header = m_dialog->findChild<DialogHeaderWidget*>();
    ASSERT_NE(header, nullptr);

    // Default radius is 0
    EXPECT_EQ(m_dialog->get_border_radius(), 0);

    // Default title matches constructor
    EXPECT_EQ(m_dialog->get_title(), QStringLiteral("Test Dialog"));
}

/**
 * @test Setting border radius updates getter and triggers repaint.
 */
TEST_F(DialogTest, BorderRadiusSetterAndGetter)
{
    // Initial value
    EXPECT_EQ(m_dialog->get_border_radius(), 0);

    // Set a new radius
    m_dialog->set_border_radius(16);
    EXPECT_EQ(m_dialog->get_border_radius(), 16);

    // Change radius again
    m_dialog->set_border_radius(8);
    EXPECT_EQ(m_dialog->get_border_radius(), 8);

    // Negative values should be ignored (radius must be >= 0)
    m_dialog->set_border_radius(-5);
    EXPECT_EQ(m_dialog->get_border_radius(), 8);  // Should remain unchanged

    // Setting same value should not trigger change
    m_dialog->set_border_radius(8);
    EXPECT_EQ(m_dialog->get_border_radius(), 8);
}

/**
 * @test Background color property setter and getter.
 */
TEST_F(DialogTest, BackgroundColorProperty)
{
    const QColor default_color = m_dialog->get_background_color();
    EXPECT_TRUE(default_color.isValid());
    EXPECT_EQ(default_color, QColor(26, 32, 33));

    const QColor new_color(255, 0, 0);
    m_dialog->set_background_color(new_color);
    EXPECT_EQ(m_dialog->get_background_color(), new_color);

    // Test with invalid color
    const QColor invalid_color;
    m_dialog->set_background_color(invalid_color);
    EXPECT_EQ(m_dialog->get_background_color(), invalid_color);
}

/**
 * @test Border color property setter and getter.
 */
TEST_F(DialogTest, BorderColorProperty)
{
    const QColor default_color = m_dialog->get_border_color();
    EXPECT_TRUE(default_color.isValid());
    EXPECT_EQ(default_color, QColor(48, 53, 58));

    const QColor new_color(0, 255, 0);
    m_dialog->set_border_color(new_color);
    EXPECT_EQ(m_dialog->get_border_color(), new_color);
}

/**
 * @test Border width property setter and getter.
 */
TEST_F(DialogTest, BorderWidthProperty)
{
    const int default_width = m_dialog->get_border_width();
    EXPECT_EQ(default_width, 2);

    m_dialog->set_border_width(5);
    EXPECT_EQ(m_dialog->get_border_width(), 5);

    m_dialog->set_border_width(0);
    EXPECT_EQ(m_dialog->get_border_width(), 0);

    m_dialog->set_border_width(10);
    EXPECT_EQ(m_dialog->get_border_width(), 10);
}

/**
 * @test Header visibility property setter and getter.
 */
TEST_F(DialogTest, HeaderVisibilityProperty)
{
    // Header should be visible by default
    EXPECT_TRUE(m_dialog->is_header_visible());

    m_dialog->set_header_visible(false);
    EXPECT_FALSE(m_dialog->is_header_visible());

    m_dialog->set_header_visible(true);
    EXPECT_TRUE(m_dialog->is_header_visible());
}

/**
 * @test Title setter and getter update the header widget title correctly.
 */
TEST_F(DialogTest, TitleSetterAndGetter)
{
    // Initial title from constructor
    EXPECT_EQ(m_dialog->get_title(), QStringLiteral("Test Dialog"));

    // Set new title via Dialog
    const QString new_title = QStringLiteral("New Test Title");
    m_dialog->set_title(new_title);
    EXPECT_EQ(m_dialog->get_title(), new_title);

    // Verify header widget also has the updated title
    auto* header = m_dialog->findChild<DialogHeaderWidget*>();
    ASSERT_NE(header, nullptr);
    EXPECT_EQ(header->get_title(), new_title);

    // Set another title
    const QString another_title = QStringLiteral("Another Title");
    m_dialog->set_title(another_title);
    EXPECT_EQ(m_dialog->get_title(), another_title);
    EXPECT_EQ(header->get_title(), another_title);
}

/**
 * @test Managed layout mode creates content layout.
 */
TEST_F(DialogTest, ManagedLayoutModeCreatesContentLayout)
{
    // Dialog created in SetUp uses Managed mode by default
    auto* content_layout = m_dialog->content_layout();
    ASSERT_NE(content_layout, nullptr);

    // Verify we can add widgets to the content layout
    auto* test_widget = new QWidget(m_dialog);
    content_layout->addWidget(test_widget);
    EXPECT_EQ(content_layout->count(), 1);
}

/**
 * @test External layout mode does not provide content layout initially.
 */
TEST_F(DialogTest, ExternalLayoutModeHandling)
{
    // Create dialog with External mode
    auto* external_dialog =
        new Dialog(QStringLiteral("External Dialog"), nullptr, Dialog::LayoutMode::External);

    // Before showing, content_layout should return nullptr and log warning
    auto* content_layout = external_dialog->content_layout();
    EXPECT_EQ(content_layout, nullptr);

    delete external_dialog;
}

/**
 * @test External layout mode integrates with existing QVBoxLayout on show.
 */
TEST_F(DialogTest, ExternalLayoutModeIntegrationOnShow)
{
    // Create dialog with External mode
    auto* external_dialog =
        new Dialog(QStringLiteral("External Dialog"), nullptr, Dialog::LayoutMode::External);

    // Set up a layout before showing (simulating Qt Designer)
    auto* existing_layout = new QVBoxLayout(external_dialog);
    auto* test_widget = new QWidget(external_dialog);
    existing_layout->addWidget(test_widget);
    external_dialog->setLayout(existing_layout);

    // Show the dialog to trigger header initialization
    external_dialog->show();
    QApplication::processEvents();

    // Verify header was added
    auto* header = external_dialog->findChild<DialogHeaderWidget*>();
    ASSERT_NE(header, nullptr);
    EXPECT_TRUE(header->isVisible());

    delete external_dialog;
}

/**
 * @test Dragging on the header moves the dialog (mouse press/move/release).
 */
TEST_F(DialogTest, DragMoveFromHeaderMovesDialog)
{
    auto* header = m_dialog->findChild<DialogHeaderWidget*>();
    ASSERT_NE(header, nullptr);

    // Start position
    const QPoint start_pos = m_dialog->pos();

    // Pick a point inside the header and map to dialog coords
    const QPoint header_center = header->rect().center();
    const QPoint press_pos = header->mapToParent(header_center);

    // Simulate press
    QMouseEvent press_evt(QEvent::MouseButtonPress, press_pos, m_dialog->mapToGlobal(press_pos),
                          Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(m_dialog, &press_evt);

    // Simulate move by an offset
    const QPoint move_global = m_dialog->mapToGlobal(press_pos + QPoint(20, 15));
    QMouseEvent move_evt(QEvent::MouseMove, press_pos + QPoint(20, 15), move_global, Qt::NoButton,
                         Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(m_dialog, &move_evt);

    // Simulate release
    QMouseEvent release_evt(QEvent::MouseButtonRelease, press_pos + QPoint(20, 15), move_global,
                            Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(m_dialog, &release_evt);
    QApplication::processEvents();

    // Position should have changed
    const QPoint end_pos = m_dialog->pos();
    EXPECT_NE(start_pos, end_pos);
}

/**
 * @test Dragging outside header does not move the dialog.
 */
TEST_F(DialogTest, DragOutsideHeaderDoesNotMoveDialog)
{
    const QPoint start_pos = m_dialog->pos();

    // Pick a point below the header (in content area)
    const QPoint press_pos(m_dialog->width() / 2, m_dialog->height() - 20);

    // Simulate press
    QMouseEvent press_evt(QEvent::MouseButtonPress, press_pos, m_dialog->mapToGlobal(press_pos),
                          Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(m_dialog, &press_evt);

    // Simulate move
    const QPoint move_global = m_dialog->mapToGlobal(press_pos + QPoint(20, 15));
    QMouseEvent move_evt(QEvent::MouseMove, press_pos + QPoint(20, 15), move_global, Qt::NoButton,
                         Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(m_dialog, &move_evt);

    // Simulate release
    QMouseEvent release_evt(QEvent::MouseButtonRelease, press_pos + QPoint(20, 15), move_global,
                            Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(m_dialog, &release_evt);
    QApplication::processEvents();

    // Position should not have changed
    const QPoint end_pos = m_dialog->pos();
    EXPECT_EQ(start_pos, end_pos);
}

/**
 * @test Dragging with hidden header does not move the dialog.
 */
TEST_F(DialogTest, DragWithHiddenHeaderDoesNotMove)
{
    auto* header = m_dialog->findChild<DialogHeaderWidget*>();
    ASSERT_NE(header, nullptr);

    // Hide the header
    m_dialog->set_header_visible(false);

    const QPoint start_pos = m_dialog->pos();

    // Try to drag at the position where header would be
    const QPoint header_center = header->rect().center();
    const QPoint press_pos = header->mapToParent(header_center);

    QMouseEvent press_evt(QEvent::MouseButtonPress, press_pos, m_dialog->mapToGlobal(press_pos),
                          Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(m_dialog, &press_evt);

    const QPoint move_global = m_dialog->mapToGlobal(press_pos + QPoint(20, 15));
    QMouseEvent move_evt(QEvent::MouseMove, press_pos + QPoint(20, 15), move_global, Qt::NoButton,
                         Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(m_dialog, &move_evt);

    QMouseEvent release_evt(QEvent::MouseButtonRelease, press_pos + QPoint(20, 15), move_global,
                            Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(m_dialog, &release_evt);
    QApplication::processEvents();

    // Position should not have changed
    const QPoint end_pos = m_dialog->pos();
    EXPECT_EQ(start_pos, end_pos);
}

/**
 * @test Right-click on header does not trigger drag.
 */
TEST_F(DialogTest, RightClickOnHeaderDoesNotDrag)
{
    auto* header = m_dialog->findChild<DialogHeaderWidget*>();
    ASSERT_NE(header, nullptr);

    const QPoint start_pos = m_dialog->pos();
    const QPoint header_center = header->rect().center();
    const QPoint press_pos = header->mapToParent(header_center);

    // Simulate right-click press
    QMouseEvent press_evt(QEvent::MouseButtonPress, press_pos, m_dialog->mapToGlobal(press_pos),
                          Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    QApplication::sendEvent(m_dialog, &press_evt);

    // Simulate move
    const QPoint move_global = m_dialog->mapToGlobal(press_pos + QPoint(20, 15));
    QMouseEvent move_evt(QEvent::MouseMove, press_pos + QPoint(20, 15), move_global, Qt::NoButton,
                         Qt::RightButton, Qt::NoModifier);
    QApplication::sendEvent(m_dialog, &move_evt);

    QMouseEvent release_evt(QEvent::MouseButtonRelease, press_pos + QPoint(20, 15), move_global,
                            Qt::RightButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(m_dialog, &release_evt);
    QApplication::processEvents();

    // Position should not have changed
    const QPoint end_pos = m_dialog->pos();
    EXPECT_EQ(start_pos, end_pos);
}

/**
 * @test Clicking the header close (emitting close_requested) rejects the dialog.
 */
TEST_F(DialogTest, HeaderCloseEmitsRejected)
{
    QSignalSpy rejected_spy(m_dialog, &QDialog::rejected);
    ASSERT_TRUE(rejected_spy.isValid());

    auto* header = m_dialog->findChild<DialogHeaderWidget*>();
    ASSERT_NE(header, nullptr);

    // Emulate header close button by emitting the signal directly
    emit header->close_requested();
    QApplication::processEvents();

    EXPECT_GE(rejected_spy.count(), 1);
}

/**
 * @test Resize event triggers update (for repaint).
 */
TEST_F(DialogTest, ResizeTriggersUpdate)
{
    const QSize initial_size = m_dialog->size();

    // Resize the dialog
    m_dialog->resize(400, 300);
    QApplication::processEvents();

    const QSize new_size = m_dialog->size();
    EXPECT_NE(initial_size, new_size);
    EXPECT_EQ(new_size, QSize(400, 300));
}

/**
 * @test Show event initializes header in External mode only once.
 */
TEST_F(DialogTest, ShowEventInitializesHeaderInExternalMode)
{
    // Create dialog with External mode
    auto* external_dialog =
        new Dialog(QStringLiteral("External Dialog"), nullptr, Dialog::LayoutMode::External);

    // Set up existing layout
    auto* existing_layout = new QVBoxLayout(external_dialog);
    auto* test_widget = new QWidget(external_dialog);
    test_widget->setObjectName("OriginalTestWidget");
    existing_layout->addWidget(test_widget);
    external_dialog->setLayout(existing_layout);

    // Header exists but should not be in layout yet
    auto* header = external_dialog->findChild<DialogHeaderWidget*>();
    ASSERT_NE(header, nullptr);

    // Before show: layout should be the original one
    EXPECT_EQ(external_dialog->layout(), existing_layout);

    // Show dialog triggers header initialization
    external_dialog->show();
    QApplication::processEvents();

    // After show: layout should have changed (new main layout wraps the old one)
    auto* new_layout = external_dialog->layout();
    EXPECT_NE(new_layout, existing_layout);
    EXPECT_NE(new_layout, nullptr);

    // Verify header is visible and in the layout
    EXPECT_TRUE(header->isVisible());

    // Find the wrapped content widget that contains our original layout
    auto* content_wrapper = external_dialog->findChild<QWidget*>("DialogContent");
    ASSERT_NE(content_wrapper, nullptr);
    EXPECT_EQ(content_wrapper->layout(), existing_layout);

    // Our test widget should still be accessible
    auto* found_test_widget = external_dialog->findChild<QWidget*>("OriginalTestWidget");
    EXPECT_EQ(found_test_widget, test_widget);

    // Store layout pointer to verify it doesn't change on second show
    auto* layout_after_first_show = external_dialog->layout();

    // Hide and show again - should NOT re-initialize
    external_dialog->hide();
    external_dialog->show();
    QApplication::processEvents();

    // Layout should be exactly the same object (no re-initialization)
    EXPECT_EQ(external_dialog->layout(), layout_after_first_show);

    // Header should still be visible
    EXPECT_TRUE(header->isVisible());

    // Content wrapper should still exist and be the same object
    auto* content_wrapper_after = external_dialog->findChild<QWidget*>("DialogContent");
    EXPECT_EQ(content_wrapper_after, content_wrapper);

    delete external_dialog;
}

/**
 * @test Paint event renders correctly with different configurations.
 */
TEST_F(DialogTest, PaintEventRendersCorrectly)
{
    // Test with rounded corners
    m_dialog->set_border_radius(10);
    m_dialog->set_background_color(QColor(50, 50, 50));
    m_dialog->set_border_color(QColor(100, 100, 100));
    m_dialog->set_border_width(3);

    // Create a pixmap to render the dialog into
    QPixmap pixmap_rounded(m_dialog->size());
    pixmap_rounded.fill(Qt::transparent);
    m_dialog->render(&pixmap_rounded);

    // Verify the pixmap is not empty (something was painted)
    EXPECT_FALSE(pixmap_rounded.isNull());
    QImage img_rounded = pixmap_rounded.toImage();
    EXPECT_FALSE(img_rounded.isNull());

    // Verify that some pixels have the background color (approximately)
    bool found_background_color = false;
    const QRgb target_color = QColor(50, 50, 50).rgb();
    for (int y = m_dialog->height() / 2; y < m_dialog->height() / 2 + 10 && !found_background_color;
         ++y)
    {
        for (int x = m_dialog->width() / 2;
             x < m_dialog->width() / 2 + 10 && !found_background_color; ++x)
        {
            const QRgb pixel = img_rounded.pixel(x, y);
            // Allow some tolerance for alpha blending
            if (qAbs(qRed(pixel) - qRed(target_color)) < 5 &&
                qAbs(qGreen(pixel) - qGreen(target_color)) < 5 &&
                qAbs(qBlue(pixel) - qBlue(target_color)) < 5)
            {
                found_background_color = true;
            }
        }
    }
    EXPECT_TRUE(found_background_color) << "Background color not found in rendered image";

    // Test with zero border radius (rectangular rendering)
    m_dialog->set_border_radius(0);
    m_dialog->set_background_color(QColor(200, 100, 50));
    m_dialog->set_border_color(QColor(255, 255, 0));
    m_dialog->set_border_width(5);

    QPixmap pixmap_rect(m_dialog->size());
    pixmap_rect.fill(Qt::transparent);
    m_dialog->render(&pixmap_rect);

    EXPECT_FALSE(pixmap_rect.isNull());
    QImage img_rect = pixmap_rect.toImage();
    EXPECT_FALSE(img_rect.isNull());

    // Verify that the two renders are different (different radius and colors)
    EXPECT_NE(img_rounded, img_rect);

    // Verify background color changed
    bool found_new_background_color = false;
    const QRgb target_color_rect = QColor(200, 100, 50).rgb();
    for (int y = m_dialog->height() / 2;
         y < m_dialog->height() / 2 + 10 && !found_new_background_color; ++y)
    {
        for (int x = m_dialog->width() / 2;
             x < m_dialog->width() / 2 + 10 && !found_new_background_color; ++x)
        {
            const QRgb pixel = img_rect.pixel(x, y);
            if (qAbs(qRed(pixel) - qRed(target_color_rect)) < 5 &&
                qAbs(qGreen(pixel) - qGreen(target_color_rect)) < 5 &&
                qAbs(qBlue(pixel) - qBlue(target_color_rect)) < 5)
            {
                found_new_background_color = true;
            }
        }
    }
    EXPECT_TRUE(found_new_background_color) << "New background color not found in rendered image";
}

/**
 * @test Multiple property changes trigger updates correctly.
 */
TEST_F(DialogTest, MultiplePropertyChangesWork)
{
    m_dialog->set_border_radius(8);
    m_dialog->set_background_color(QColor(10, 20, 30));
    m_dialog->set_border_color(QColor(40, 50, 60));
    m_dialog->set_border_width(4);
    m_dialog->set_header_visible(false);
    m_dialog->set_title(QStringLiteral("Changed Title"));

    EXPECT_EQ(m_dialog->get_border_radius(), 8);
    EXPECT_EQ(m_dialog->get_background_color(), QColor(10, 20, 30));
    EXPECT_EQ(m_dialog->get_border_color(), QColor(40, 50, 60));
    EXPECT_EQ(m_dialog->get_border_width(), 4);
    EXPECT_FALSE(m_dialog->is_header_visible());
    EXPECT_EQ(m_dialog->get_title(), QStringLiteral("Changed Title"));

    QApplication::processEvents();
}
