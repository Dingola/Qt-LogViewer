/**
 * @file TableViewTest.cpp
 * @brief Implements tests for the TableView class (hover color and hover index signaling).
 */

#include "Qt-LogViewer/Views/Shared/TableViewTest.h"

#include <QApplication>
#include <QMouseEvent>
#include <QSignalSpy>
#include <QStandardItemModel>

#include "Qt-LogViewer/Views/Shared/TableView.h"

/**
 * @brief Creates and populates a simple model (rows x cols) with textual items.
 * @param rows Number of rows to create.
 * @param cols Number of columns to create.
 * @return The created model pointer (owned by the test fixture).
 */
auto TableViewTest::create_model(int rows, int cols) -> QStandardItemModel*
{
    m_model = new QStandardItemModel(rows, cols, m_view);

    for (int r = 0; r < rows; ++r)
    {
        for (int c = 0; c < cols; ++c)
        {
            auto* item = new QStandardItem(QStringLiteral("R%1C%2").arg(r).arg(c));
            m_model->setItem(r, c, item);
        }
    }

    return m_model;
}

/**
 * @brief GoogleTest SetUp hook. Creates the TableView and attaches a model.
 *
 * Enables mouse tracking so hover events are generated without a mouse button pressed.
 */
void TableViewTest::SetUp()
{
    m_view = new TableView();
    m_view->setMouseTracking(true);
    create_model(5, 3);
    m_view->setModel(m_model);
    m_view->resize(400, 200);
    m_view->show();
    QApplication::processEvents();
}

/**
 * @brief GoogleTest TearDown hook. Destroys the TableView and releases the model pointer.
 */
void TableViewTest::TearDown()
{
    delete m_view;
    m_view = nullptr;
    m_model = nullptr;
}

/**
 * @test Verifies the default hover row color is the expected value defined in TableView.
 */
TEST_F(TableViewTest, DefaultHoverColorIsSet)
{
    QColor color = m_view->get_hover_row_color();
    EXPECT_EQ(color.name().toLower(), QStringLiteral("#25384a"));
}

/**
 * @test Verifies that setting the hover row color updates the stored value.
 */
TEST_F(TableViewTest, SetHoverColorChangesValue)
{
    QColor new_color("#112233");
    m_view->set_hover_row_color(new_color);
    QColor read_back = m_view->get_hover_row_color();
    EXPECT_EQ(read_back, new_color);
}

/**
 * @test Verifies that moving the mouse over a cell emits hover_index_changed with that index.
 *
 * The event is sent to the viewport (child of QTableView) to trigger the overridden mouseMoveEvent.
 */
TEST_F(TableViewTest, HoverIndexChangedOnMouseMove)
{
    QSignalSpy spy(m_view, &TableView::hover_index_changed);
    ASSERT_TRUE(spy.isValid());

    QModelIndex target_index = m_model->index(2, 1);
    QRect rect = m_view->visualRect(target_index);
    QPoint pt = rect.center();

    QMouseEvent move_event(QEvent::MouseMove, pt, Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(m_view->viewport(), &move_event);
    QApplication::processEvents();

    ASSERT_GE(spy.count(), 1);

    QVariant last_arg = spy.takeLast().at(0);
    QModelIndex emitted_index = last_arg.value<QModelIndex>();

    EXPECT_TRUE(emitted_index.isValid());
    EXPECT_EQ(emitted_index.row(), target_index.row());
    EXPECT_EQ(emitted_index.column(), target_index.column());
}

/**
 * @test Verifies that leaving the view emits an invalid hover index (reset condition).
 *
 * A valid hover emission is generated first, followed by a leave event.
 */
TEST_F(TableViewTest, HoverIndexResetOnLeave)
{
    QSignalSpy spy(m_view, &TableView::hover_index_changed);
    ASSERT_TRUE(spy.isValid());

    QModelIndex target_index = m_model->index(0, 0);
    QRect rect = m_view->visualRect(target_index);
    QPoint inside = rect.center();

    QMouseEvent move_event(QEvent::MouseMove, inside, Qt::NoButton, Qt::NoButton, Qt::NoModifier);
    QApplication::sendEvent(m_view->viewport(), &move_event);
    QApplication::processEvents();

    QEvent leave_event(QEvent::Leave);
    QApplication::sendEvent(m_view, &leave_event);
    QApplication::processEvents();

    ASSERT_GE(spy.count(), 2);

    QVariant last_arg = spy.takeLast().at(0);
    QModelIndex emitted_index = last_arg.value<QModelIndex>();
    EXPECT_FALSE(emitted_index.isValid());
}
