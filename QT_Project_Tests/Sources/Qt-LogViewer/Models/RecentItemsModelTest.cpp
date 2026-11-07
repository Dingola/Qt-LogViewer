#include "Qt-LogViewer/Models/RecentItemsModelTest.h"

#include <QDateTime>
#include <QSignalSpy>
#include <QString>

#include "Qt-LogViewer/Models/RecentItemsModel.h"
#include "Qt-LogViewer/Models/RecentRoles.h"

/**
 * @brief Verifies semantic roles and standard Qt roles expose typed presentation values.
 */
TEST_F(RecentItemsModelTest, ExposesPresentationThroughStableRoles)
{
    const QDateTime last_used =
        QDateTime::fromString(QStringLiteral("2025-03-10T12:34:56"), Qt::ISODate);
    RecentItemPresentation item;
    item.identity = QStringLiteral("item-id");
    item.title = QStringLiteral("Recent item");
    item.secondary_text = QStringLiteral("Supporting text");
    item.last_used = last_used;
    item.status_text = QStringLiteral("Available");
    item.tooltip = QStringLiteral("Detailed information");

    RecentItemsModel model;
    model.set_items({item});
    const QModelIndex index = model.index(0, 0);

    ASSERT_TRUE(index.isValid());
    EXPECT_EQ(index.data(Qt::DisplayRole).toString(), item.title);
    EXPECT_EQ(index.data(to_role_id(RecentItemRole::Identity)).toString(),
              item.identity.toString());
    EXPECT_EQ(index.data(to_role_id(RecentItemRole::SecondaryText)).toString(),
              item.secondary_text);
    EXPECT_EQ(index.data(to_role_id(RecentItemRole::LastUsed)).toDateTime(), last_used);
    EXPECT_EQ(index.data(to_role_id(RecentItemRole::StatusText)).toString(), item.status_text);
    EXPECT_EQ(index.data(Qt::ToolTipRole).toString(), item.tooltip);
    EXPECT_EQ(index.data(Qt::AccessibleDescriptionRole), index.data(Qt::ToolTipRole));
}

/**
 * @brief Verifies replacement emits a reset and atomically changes the visible items.
 */
TEST_F(RecentItemsModelTest, ResetsWhenItemsAreReplaced)
{
    RecentItemsModel model;
    QSignalSpy reset_spy(&model, &QAbstractItemModel::modelReset);
    RecentItemPresentation first;
    first.title = QStringLiteral("First");
    RecentItemPresentation second;
    second.title = QStringLiteral("Second");

    model.set_items({first, second});

    EXPECT_EQ(reset_spy.count(), 1);
    EXPECT_EQ(model.rowCount(), 2);
    EXPECT_EQ(model.index(1, 0).data(Qt::DisplayRole).toString(), second.title);
}

/**
 * @brief Verifies disabled presentations remain visible but cannot be activated.
 */
TEST_F(RecentItemsModelTest, RepresentsDisabledItemsThroughFlags)
{
    RecentItemPresentation item;
    item.title = QStringLiteral("Unavailable item");
    item.enabled = false;
    item.tooltip = QStringLiteral("The item is unavailable");
    RecentItemsModel model;
    model.set_items({item});

    const QModelIndex index = model.index(0, 0);
    EXPECT_TRUE(index.flags().testFlag(Qt::ItemIsSelectable));
    EXPECT_FALSE(index.flags().testFlag(Qt::ItemIsEnabled));
    EXPECT_EQ(index.data(Qt::ToolTipRole).toString(), item.tooltip);
}

/**
 * @brief Verifies clearing the list preserves valid list-model semantics.
 */
TEST_F(RecentItemsModelTest, ClearsPresentationItems)
{
    RecentItemPresentation item;
    item.title = QStringLiteral("Recent item");
    RecentItemsModel model;
    model.set_items({item});
    QSignalSpy reset_spy(&model, &QAbstractItemModel::modelReset);

    model.clear();

    EXPECT_EQ(reset_spy.count(), 1);
    EXPECT_EQ(model.rowCount(), 0);
    EXPECT_FALSE(model.index(0, 1).isValid());
}
