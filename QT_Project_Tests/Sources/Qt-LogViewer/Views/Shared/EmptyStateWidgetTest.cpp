/**
 * @file EmptyStateWidgetTest.cpp
 * @brief Implements focused tests for EmptyStateWidget.
 */

#include "Qt-LogViewer/Views/Shared/EmptyStateWidgetTest.h"

#include <QAction>
#include <QLabel>
#include <QPushButton>
#include <QSignalSpy>

#include "Qt-LogViewer/Views/Shared/EmptyStateWidget.h"

/** @test Verifies that context actions remain the source of button state and
 * triggering. */
TEST_F(EmptyStateWidgetTest, MirrorsContextAction)
{
    EmptyStateWidget widget;
    QAction action(QStringLiteral("Try Again"), &widget);
    QSignalSpy triggered_spy(&action, &QAction::triggered);

    widget.add_action(&action, EmptyStateWidget::ActionRole::Primary);
    QPushButton* button = widget.findChild<QPushButton*>();
    ASSERT_NE(button, nullptr);
    EXPECT_EQ(button->text(), QStringLiteral("Try Again"));
    EXPECT_EQ(button->property("actionRole").toString(), QStringLiteral("primary"));

    action.setEnabled(false);
    EXPECT_FALSE(button->isEnabled());
    action.setEnabled(true);
    button->click();
    EXPECT_EQ(triggered_spy.count(), 1);
}

/** @test Verifies that hosts can describe drag-and-drop state without mime-data
 * coupling. */
TEST_F(EmptyStateWidgetTest, PresentsHostControlledDropState)
{
    EmptyStateWidget widget;
    widget.set_drop_text(QStringLiteral("Drop files here"));
    widget.set_drop_space_reserved(true);
    widget.set_drop_state(EmptyStateWidget::DropState::Accepting);

    const QLabel* drop_hint = widget.findChild<QLabel*>(QStringLiteral("emptyStateDropHint"));
    ASSERT_NE(drop_hint, nullptr);
    EXPECT_TRUE(drop_hint->isVisibleTo(&widget));
    EXPECT_EQ(drop_hint->text(), QStringLiteral("Drop files here"));
    EXPECT_EQ(drop_hint->property("dropState").toString(), QStringLiteral("accepting"));
    EXPECT_TRUE(drop_hint->sizePolicy().retainSizeWhenHidden());
}
