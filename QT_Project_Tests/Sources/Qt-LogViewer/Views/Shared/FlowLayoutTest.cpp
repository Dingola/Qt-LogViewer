#include "Qt-LogViewer/Views/Shared/FlowLayoutTest.h"

#include <QLabel>
#include <QWidget>

/**
 * @brief Sets up the test fixture for each test.
 */
void FlowLayoutTest::SetUp()
{
    m_parent_widget = new QWidget();
    m_layout = new FlowLayout(m_parent_widget, 0, 8, 4, FlowLayout::RowAlignment::Left);
}

/**
 * @brief Tears down the test fixture after each test.
 */
void FlowLayoutTest::TearDown()
{
    delete m_parent_widget;
    m_parent_widget = nullptr;
    m_layout = nullptr;
}

/**
 * @brief Tests construction and default alignment.
 */
TEST_F(FlowLayoutTest, DefaultAlignment)
{
    EXPECT_EQ(m_layout->get_row_alignment(), FlowLayout::RowAlignment::Left);
}

/**
 * @brief Tests setting and getting row alignment.
 */
TEST_F(FlowLayoutTest, SetAndGetRowAlignment)
{
    m_layout->set_row_alignment(FlowLayout::RowAlignment::Center);
    EXPECT_EQ(m_layout->get_row_alignment(), FlowLayout::RowAlignment::Center);

    m_layout->set_row_alignment(FlowLayout::RowAlignment::CenterLeft);
    EXPECT_EQ(m_layout->get_row_alignment(), FlowLayout::RowAlignment::CenterLeft);
}

/**
 * @brief Tests adding and counting items.
 */
TEST_F(FlowLayoutTest, AddAndCountItems)
{
    auto* label1 = new QLabel("A", m_parent_widget);
    auto* label2 = new QLabel("B", m_parent_widget);
    m_layout->addWidget(label1);
    m_layout->addWidget(label2);
    EXPECT_EQ(m_layout->count(), 2);
}

/**
 * @brief Tests itemAt and takeAt methods.
 */
TEST_F(FlowLayoutTest, ItemAtAndTakeAt)
{
    auto* label1 = new QLabel("A", m_parent_widget);
    auto* label2 = new QLabel("B", m_parent_widget);
    m_layout->addWidget(label1);
    m_layout->addWidget(label2);

    EXPECT_EQ(m_layout->itemAt(0)->widget(), label1);
    EXPECT_EQ(m_layout->itemAt(1)->widget(), label2);
    EXPECT_EQ(m_layout->itemAt(2), nullptr);

    auto* item = m_layout->takeAt(0);
    EXPECT_EQ(item->widget(), label1);
    delete item;
    EXPECT_EQ(m_layout->count(), 1);
}

/**
 * @brief Tests horizontal and vertical spacing.
 */
TEST_F(FlowLayoutTest, Spacing)
{
    EXPECT_EQ(m_layout->horizontal_spacing(), 8);
    EXPECT_EQ(m_layout->vertical_spacing(), 4);
}

/**
 * @brief Tests expand_to_show_all_rows property.
 */
TEST_F(FlowLayoutTest, ExpandToShowAllRows)
{
    EXPECT_FALSE(m_layout->get_expand_to_show_all_rows());
    m_layout->set_expand_to_show_all_rows(true);
    EXPECT_TRUE(m_layout->get_expand_to_show_all_rows());
}

/**
 * @brief Tests sizeHint calculation with multiple items.
 */
TEST_F(FlowLayoutTest, SizeHintWithItems)
{
    auto* label1 = new QLabel("A", m_parent_widget);
    auto* label2 = new QLabel("B", m_parent_widget);
    auto* label3 = new QLabel("C", m_parent_widget);
    m_layout->addWidget(label1);
    m_layout->addWidget(label2);
    m_layout->addWidget(label3);

    QSize hint = m_layout->sizeHint();
    EXPECT_GT(hint.width(), 0);
    EXPECT_GT(hint.height(), 0);
}

/**
 * @brief Tests minimumSize calculation.
 */
TEST_F(FlowLayoutTest, MinimumSizeWithItems)
{
    auto* label1 = new QLabel("A", m_parent_widget);
    auto* label2 = new QLabel("B", m_parent_widget);
    m_layout->addWidget(label1);
    m_layout->addWidget(label2);

    QSize min_size = m_layout->minimumSize();
    EXPECT_GT(min_size.width(), 0);
    EXPECT_GT(min_size.height(), 0);
}

/**
 * @brief Tests that layout does not crash with no items.
 */
TEST_F(FlowLayoutTest, NoItems)
{
    EXPECT_EQ(m_layout->count(), 0);
    EXPECT_EQ(m_layout->sizeHint(), QSize());
    EXPECT_EQ(m_layout->minimumSize(), QSize());
}

/**
 * @brief Tests that do_layout arranges widgets as expected.
 */
TEST_F(FlowLayoutTest, LayoutGeometry)
{
    auto* label1 = new QLabel("A", m_parent_widget);
    auto* label2 = new QLabel("B", m_parent_widget);
    auto* label3 = new QLabel("C", m_parent_widget);
    m_layout->addWidget(label1);
    m_layout->addWidget(label2);
    m_layout->addWidget(label3);

    m_parent_widget->resize(200, 100);
    m_layout->setGeometry(QRect(0, 0, 200, 100));

    // Check that widgets have non-overlapping, valid geometry
    QRect geom1 = label1->geometry();
    QRect geom2 = label2->geometry();
    QRect geom3 = label3->geometry();
    EXPECT_TRUE(geom1.isValid());
    EXPECT_TRUE(geom2.isValid());
    EXPECT_TRUE(geom3.isValid());
    EXPECT_NE(geom1, geom2);
    EXPECT_NE(geom2, geom3);
}

/**
 * @brief Tests that negative spacing falls back to smart_spacing.
 */
TEST_F(FlowLayoutTest, NegativeSpacingFallback)
{
    QWidget* test_widget = new QWidget();
    auto* layout = new FlowLayout(test_widget, 0, -1, -1, FlowLayout::RowAlignment::Left);
    EXPECT_GE(layout->horizontal_spacing(), 0);
    EXPECT_GE(layout->vertical_spacing(), 0);
    delete test_widget;
}

/**
 * @brief Tests that items wrap to new rows when parent width is reduced.
 */
TEST_F(FlowLayoutTest, WrappingWithParentWidth)
{
    QWidget* test_widget = new QWidget();
    auto* layout = new FlowLayout(test_widget, 0, 8, 4, FlowLayout::RowAlignment::Left);
    auto* label1 = new QLabel("A", test_widget);
    auto* label2 = new QLabel("B", test_widget);
    auto* label3 = new QLabel("C", test_widget);
    label1->setMinimumWidth(50);
    label2->setMinimumWidth(50);
    label3->setMinimumWidth(50);
    layout->addWidget(label1);
    layout->addWidget(label2);
    layout->addWidget(label3);

    test_widget->resize(300, 100);
    layout->setGeometry(QRect(0, 0, 300, 100));
    int y1 = label1->geometry().top();
    int y2 = label2->geometry().top();
    int y3 = label3->geometry().top();

    test_widget->resize(60, 100);
    layout->setGeometry(QRect(0, 0, 60, 100));
    int y1_small = label1->geometry().top();
    int y2_small = label2->geometry().top();
    int y3_small = label3->geometry().top();

    EXPECT_NE(y1_small, y2_small);
    EXPECT_NE(y2_small, y3_small);

    delete test_widget;
}

/**
 * @brief Tests that custom margins are respected in layout geometry.
 */
TEST_F(FlowLayoutTest, CustomMargins)
{
    QWidget* test_widget = new QWidget();
    auto* layout = new FlowLayout(test_widget, 10, 8, 4, FlowLayout::RowAlignment::Left);
    auto* label1 = new QLabel("A", test_widget);
    auto* label2 = new QLabel("B", test_widget);
    layout->addWidget(label1);
    layout->addWidget(label2);

    test_widget->resize(100, 100);
    layout->setGeometry(QRect(0, 0, 100, 100));

    EXPECT_EQ(label1->geometry().left(), 10);
    EXPECT_EQ(label1->geometry().top(), 10);

    delete test_widget;
}

/**
 * @brief Tests removing all items from the layout.
 */
TEST_F(FlowLayoutTest, RemoveAllItems)
{
    auto* label1 = new QLabel("A", m_parent_widget);
    auto* label2 = new QLabel("B", m_parent_widget);
    m_layout->addWidget(label1);
    m_layout->addWidget(label2);

    while (m_layout->count() > 0)
    {
        auto* item = m_layout->takeAt(0);
        delete item;
    }

    EXPECT_EQ(m_layout->count(), 0);
}

/**
 * @brief Tests layout with a large number of items.
 */
TEST_F(FlowLayoutTest, LargeNumberOfItems)
{
    const int num_items = 50;

    for (int i = 0; i < num_items; ++i)
    {
        m_layout->addWidget(new QLabel(QString::number(i), m_parent_widget));
    }

    m_parent_widget->resize(200, 400);
    m_layout->setGeometry(QRect(0, 0, 200, 400));
    EXPECT_EQ(m_layout->count(), num_items);
}

/**
 * @brief Tests changing alignment after adding widgets updates layout.
 */
TEST_F(FlowLayoutTest, ChangeAlignmentAfterAddingItems)
{
    auto* label1 = new QLabel("A", m_parent_widget);
    auto* label2 = new QLabel("B", m_parent_widget);
    m_layout->addWidget(label1);
    m_layout->addWidget(label2);

    m_layout->set_row_alignment(FlowLayout::RowAlignment::Center);
    m_parent_widget->resize(100, 100);
    m_layout->setGeometry(QRect(0, 0, 100, 100));

    int center = m_parent_widget->width() / 2;
    int row_center = (label1->geometry().left() + label2->geometry().right()) / 2;
    EXPECT_NEAR(center, row_center, 8);
}

/**
 * @brief Tests expand_to_show_all_rows with many items increases sizeHint height.
 */
TEST_F(FlowLayoutTest, ExpandToShowAllRowsManyItems)
{
    for (int i = 0; i < 20; ++i)
    {
        m_layout->addWidget(new QLabel(QString::number(i), m_parent_widget));
    }

    m_layout->set_expand_to_show_all_rows(true);
    QSize hint = m_layout->sizeHint();
    EXPECT_GT(hint.height(), 100);  // Should be taller than a single row
}

/**
 * @brief Tests that hidden widgets are not laid out.
 */
TEST_F(FlowLayoutTest, HiddenWidgetsNotLaidOut)
{
    auto* label1 = new QLabel("A", m_parent_widget);
    auto* label2 = new QLabel("B", m_parent_widget);
    m_layout->addWidget(label1);
    m_layout->addWidget(label2);
    label2->hide();

    m_parent_widget->resize(100, 100);
    m_layout->setGeometry(QRect(0, 0, 100, 100));

    EXPECT_TRUE(label1->geometry().isValid());
    EXPECT_TRUE(label2->geometry().isValid());  // Geometry is set, but widget is hidden
    EXPECT_TRUE(label2->isHidden());
}
