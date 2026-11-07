/**
 * @file RecentItemsMenuBinderTest.cpp
 * @brief Tests model synchronization and semantic signals of the recent-items menu binder.
 */

#include "Qt-LogViewer/Views/Shared/RecentItemsMenuBinderTest.h"

#include <QAction>
#include <QMenu>
#include <QSignalSpy>

#include "Qt-LogViewer/Models/RecentItemsModel.h"
#include "Qt-LogViewer/Models/RecentRoles.h"
#include "Qt-LogViewer/Views/Shared/RecentItemsMenuBinder.h"

namespace
{
/**
 * @brief Creates the semantic role configuration used by the production recent-items model.
 * @return Role configuration for binder tests.
 */
[[nodiscard]] auto make_roles() -> RecentItemsMenuRoles
{
    const RecentItemsMenuRoles roles{
        to_role_id(RecentItemRole::Title),   to_role_id(RecentItemRole::Identity),
        to_role_id(RecentItemRole::Enabled), to_role_id(RecentItemRole::ToolTip),
        to_role_id(RecentItemRole::Icon),    to_role_id(RecentItemRole::StatusText)};
    return roles;
}
}  // namespace

/**
 * @brief Verifies generated actions expose model data and semantic activation identities.
 */
TEST_F(RecentItemsMenuBinderTest, MirrorsItemsAndEmitsActivationIdentity)
{
    RecentItemPresentation available;
    available.identity = QStringLiteral("available-id");
    available.title = QStringLiteral("Available item");
    available.tooltip = QStringLiteral("Open the available item");
    RecentItemPresentation unavailable;
    unavailable.identity = QStringLiteral("missing-id");
    unavailable.title = QStringLiteral("Missing item");
    unavailable.enabled = false;
    unavailable.status_text = QStringLiteral("Not found");

    RecentItemsModel model;
    model.set_items({available, unavailable});
    QMenu menu;
    RecentItemsMenuBinder binder(&menu, &model, make_roles());
    binder.set_disabled_text_format(QStringLiteral("%1 — %2"));
    QSignalSpy activated_spy(&binder, &RecentItemsMenuBinder::item_activated);

    const QList<QAction*> actions = menu.actions();
    ASSERT_EQ(actions.size(), 2);
    EXPECT_EQ(actions.at(0)->text(), available.title);
    EXPECT_EQ(actions.at(0)->toolTip(), available.tooltip);
    EXPECT_FALSE(actions.at(1)->isEnabled());
    EXPECT_EQ(actions.at(1)->text(), QStringLiteral("Missing item — Not found"));

    actions.at(0)->trigger();
    ASSERT_EQ(activated_spy.count(), 1);
    EXPECT_EQ(activated_spy.at(0).at(0).toString(), available.identity.toString());
}

/**
 * @brief Verifies model resets, visible limits, empty text, and clear intent are handled locally.
 */
TEST_F(RecentItemsMenuBinderTest, ReactsToModelChangesAndOptionalActions)
{
    RecentItemsModel model;
    QMenu menu;
    QAction footer_action(QStringLiteral("Footer"));
    RecentItemsMenuBinder binder(&menu, &model, make_roles());
    binder.set_maximum_visible_items(1);
    binder.set_empty_text(QStringLiteral("No recent items"));
    binder.set_clear_text(QStringLiteral("Clear recent items"));
    binder.set_footer_actions({&footer_action});
    QSignalSpy clear_spy(&binder, &RecentItemsMenuBinder::clear_requested);
    QSignalSpy disabled_spy(&binder, &RecentItemsMenuBinder::contains_disabled_items_changed);

    ASSERT_EQ(menu.actions().size(), 4);
    EXPECT_EQ(menu.actions().at(0)->text(), QStringLiteral("No recent items"));
    EXPECT_FALSE(menu.actions().at(0)->isEnabled());
    EXPECT_FALSE(menu.actions().constLast()->isEnabled());

    RecentItemPresentation first;
    first.title = QStringLiteral("First");
    RecentItemPresentation second;
    second.title = QStringLiteral("Second");
    second.enabled = false;
    model.set_items({first, second});

    ASSERT_EQ(menu.actions().size(), 4);
    EXPECT_EQ(menu.actions().at(0)->text(), first.title);
    EXPECT_EQ(menu.actions().at(2), &footer_action);
    EXPECT_TRUE(menu.actions().constLast()->isEnabled());
    ASSERT_FALSE(disabled_spy.isEmpty());
    EXPECT_TRUE(disabled_spy.constLast().at(0).toBool());
    menu.actions().constLast()->trigger();
    EXPECT_EQ(clear_spy.count(), 1);
}
