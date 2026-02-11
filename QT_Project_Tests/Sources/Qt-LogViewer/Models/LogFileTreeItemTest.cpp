#include "Qt-LogViewer/Models/LogFileTreeItemTest.h"

#include "Qt-LogViewer/Models/LogFileTreeItem.h"

/**
 * @brief Sets up the test fixture for each test.
 */
void LogFileTreeItemTest::SetUp() {}

/**
 * @brief Tears down the test fixture after each test.
 */
void LogFileTreeItemTest::TearDown() {}

/**
 * @brief Tests default construction and column/data access.
 */
TEST_F(LogFileTreeItemTest, DefaultConstructionAndDataAccess)
{
    QVector<QVariant> data;
    data << "Group" << 42;
    LogFileTreeItem item(data);

    EXPECT_EQ(item.column_count(), 2);
    EXPECT_EQ(item.data(0).toString(), "Group");
    EXPECT_EQ(item.data(1).toInt(), 42);
    EXPECT_EQ(item.data(2), QVariant());  // Out of bounds
}

/**
 * @brief Tests child management: append_child, child, child_count.
 */
TEST_F(LogFileTreeItemTest, ChildManagement)
{
    QVector<QVariant> parent_data;
    parent_data << "Parent";
    LogFileTreeItem parent_item(parent_data);

    QVector<QVariant> child_data;
    child_data << "Child";
    auto* child_item = new LogFileTreeItem(child_data, &parent_item);

    EXPECT_EQ(parent_item.child_count(), 0);
    parent_item.append_child(child_item);
    EXPECT_EQ(parent_item.child_count(), 1);
    EXPECT_EQ(parent_item.child(0), child_item);
    EXPECT_EQ(parent_item.child(1), nullptr);  // Out of bounds
}

/**
 * @brief Tests child() with negative index.
 */
TEST_F(LogFileTreeItemTest, ChildNegativeIndex)
{
    LogFileTreeItem parent(QVector<QVariant>{"Parent"});
    auto* child = new LogFileTreeItem(QVector<QVariant>{"Child"}, &parent);
    parent.append_child(child);

    EXPECT_EQ(parent.child(-1), nullptr);
}

/**
 * @brief Tests parent_item and row logic.
 */
TEST_F(LogFileTreeItemTest, ParentAndRowLogic)
{
    QVector<QVariant> root_data;
    root_data << "Root";
    LogFileTreeItem root_item(root_data);

    QVector<QVariant> child1_data;
    child1_data << "Child1";
    auto* child1 = new LogFileTreeItem(child1_data, &root_item);

    QVector<QVariant> child2_data;
    child2_data << "Child2";
    auto* child2 = new LogFileTreeItem(child2_data, &root_item);

    root_item.append_child(child1);
    root_item.append_child(child2);

    EXPECT_EQ(child1->parent_item(), &root_item);
    EXPECT_EQ(child2->parent_item(), &root_item);

    EXPECT_EQ(child1->row(), 0);
    EXPECT_EQ(child2->row(), 1);
    EXPECT_EQ(root_item.row(), 0);  // Root has no parent
}

/**
 * @brief Tests destructor deletes children recursively.
 */
TEST_F(LogFileTreeItemTest, DestructorDeletesChildren)
{
    auto* root = new LogFileTreeItem(QVector<QVariant>{"Root"});
    auto* child1 = new LogFileTreeItem(QVector<QVariant>{"Child1"}, root);
    auto* child2 = new LogFileTreeItem(QVector<QVariant>{"Child2"}, root);
    root->append_child(child1);
    root->append_child(child2);

    // No explicit check, but should not leak or crash on delete
    delete root;
}

/**
 * @brief Tests remove_child with out-of-bounds indices.
 */
TEST_F(LogFileTreeItemTest, RemoveChildOutOfBounds)
{
    LogFileTreeItem parent(QVector<QVariant>{"Parent"});
    auto* child = new LogFileTreeItem(QVector<QVariant>{"Child"}, &parent);
    parent.append_child(child);

    // Negative index
    EXPECT_EQ(parent.remove_child(-1), nullptr);
    EXPECT_EQ(parent.child_count(), 1);

    // Index >= child_count
    EXPECT_EQ(parent.remove_child(1), nullptr);
    EXPECT_EQ(parent.child_count(), 1);
}

/**
 * @brief Tests multi-column item data access.
 */
TEST_F(LogFileTreeItemTest, MultiColumnDataAccess)
{
    QVector<QVariant> data;
    data << "Col1" << "Col2" << 123;
    LogFileTreeItem item(data);

    EXPECT_EQ(item.column_count(), 3);
    EXPECT_EQ(item.data(0).toString(), "Col1");
    EXPECT_EQ(item.data(1).toString(), "Col2");
    EXPECT_EQ(item.data(2).toInt(), 123);
}

/**
 * @brief Tests data() with negative index.
 */
TEST_F(LogFileTreeItemTest, DataNegativeIndex)
{
    QVector<QVariant> data;
    data << "A" << 123;
    LogFileTreeItem item(data);

    EXPECT_EQ(item.data(-1), QVariant());
}

/**
 * @brief Tests row indices after removing a child.
 */
TEST_F(LogFileTreeItemTest, RowAfterRemoveChild)
{
    LogFileTreeItem parent(QVector<QVariant>{"Parent"});
    auto* child1 = new LogFileTreeItem(QVector<QVariant>{"Child1"}, &parent);
    auto* child2 = new LogFileTreeItem(QVector<QVariant>{"Child2"}, &parent);
    parent.append_child(child1);
    parent.append_child(child2);

    EXPECT_EQ(child1->row(), 0);
    EXPECT_EQ(child2->row(), 1);

    // Remove first child
    auto* removed = parent.remove_child(0);
    EXPECT_EQ(removed, child1);
    EXPECT_EQ(parent.child_count(), 1);
    EXPECT_EQ(parent.child(0), child2);
    EXPECT_EQ(child2->row(), 0);

    delete removed;
}

/**
 * @brief Tests row() when parent has no children.
 */
TEST_F(LogFileTreeItemTest, RowWithParentNoChildren)
{
    QVector<QVariant> parent_data;
    parent_data << "Parent";
    LogFileTreeItem parent_item(parent_data);

    QVector<QVariant> child_data;
    child_data << "Child";
    auto* child_item = new LogFileTreeItem(child_data, &parent_item);

    EXPECT_EQ(child_item->row(), 0);

    delete child_item;
}
