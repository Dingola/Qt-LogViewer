/**
 * @file DialogTest.cpp
 * @brief Implements tests for the Dialog base class (header, drag-move, border radius/mask, close).
 */

#include "Qt-LogViewer/Views/Shared/DialogTest.h"

#include <QApplication>
#include <QMouseEvent>
#include <QRegion>
#include <QSignalSpy>
#include <QWidget>

#include "Qt-LogViewer/Views/Shared/Dialog.h"
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

    // Header exists (access via QObject hierarchy; header_widget() is protected)
    auto* header = m_dialog->findChild<DialogHeaderWidget*>();
    ASSERT_NE(header, nullptr);

    // Default radius
    EXPECT_EQ(m_dialog->get_border_radius(), 16);
}

/**
 * @test Setting border radius updates getter and affects the window mask on resize.
 */
TEST_F(DialogTest, BorderRadiusAffectsMaskOnResize)
{
    // Initial mask after a resize
    m_dialog->resize(320, 180);
    QApplication::processEvents();
    const QRegion mask_before = m_dialog->mask();
    EXPECT_FALSE(mask_before.isEmpty());

    // Change radius and resize again to force mask recompute
    m_dialog->set_border_radius(4);
    m_dialog->resize(321, 181);
    QApplication::processEvents();

    const QRegion mask_after = m_dialog->mask();
    EXPECT_FALSE(mask_after.isEmpty());

    // Regions should differ when radius changes (shape changes)
    EXPECT_NE(mask_before, mask_after);
    EXPECT_EQ(m_dialog->get_border_radius(), 4);
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
