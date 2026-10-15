#include "Qt-LogViewer/Controllers/WorkspaceShortcutControllerTest.h"

#include <QKeySequence>
#include <QShortcut>
#include <QWidget>

#include "Qt-LogViewer/Controllers/WorkspaceShortcutController.h"
#include "Qt-LogViewer/Views/App/LogDetailsWidget.h"
#include "Qt-LogViewer/Views/App/LogFilterBarWidget.h"
#include "Qt-LogViewer/Views/App/LogTabWidget.h"

/**
 * @file WorkspaceShortcutControllerTest.cpp
 * @brief Implements focused tests for workspace keyboard coordination.
 */

/** @test Verifies shortcut registration, cyclic tab navigation, and standard tab closing. */
TEST_F(WorkspaceShortcutControllerTest, ProvidesEssentialWorkspaceShortcuts)
{
    QWidget window;
    auto* tab_widget = new LogTabWidget(&window);
    auto* filter_bar = new LogFilterBarWidget(&window);
    auto* details_widget = new LogDetailsWidget(&window);
    WorkspaceShortcutController controller(nullptr, tab_widget, filter_bar, details_widget,
                                           &window);
    tab_widget->addTab(new QWidget(), QStringLiteral("First"));
    tab_widget->addTab(new QWidget(), QStringLiteral("Second"));
    tab_widget->setCurrentIndex(1);

    /** @brief Finds one registered shortcut by its stable object name. */
    const auto shortcut = [tab_widget](const QString& object_name) {
        QShortcut* result = tab_widget->findChild<QShortcut*>(object_name);
        return result;
    };

    QShortcut* focus_search = shortcut(QStringLiteral("shortcutFocusSearch"));
    QShortcut* clear_search = shortcut(QStringLiteral("shortcutClearSearch"));
    QShortcut* next_result = shortcut(QStringLiteral("shortcutNextSearchResult"));
    QShortcut* previous_result = shortcut(QStringLiteral("shortcutPreviousSearchResult"));
    QShortcut* copy_entry = shortcut(QStringLiteral("shortcutCopyLogEntry"));
    QShortcut* close_view = shortcut(QStringLiteral("shortcutCloseCurrentView"));
    QShortcut* next_tab = shortcut(QStringLiteral("shortcutNextViewTab"));
    QShortcut* previous_tab = shortcut(QStringLiteral("shortcutPreviousViewTab"));

    ASSERT_NE(focus_search, nullptr);
    ASSERT_NE(clear_search, nullptr);
    ASSERT_NE(next_result, nullptr);
    ASSERT_NE(previous_result, nullptr);
    ASSERT_NE(copy_entry, nullptr);
    ASSERT_NE(close_view, nullptr);
    ASSERT_NE(next_tab, nullptr);
    ASSERT_NE(previous_tab, nullptr);
    EXPECT_EQ(focus_search->key(), QKeySequence::Find);
    EXPECT_EQ(clear_search->key(), QKeySequence(Qt::Key_Escape));
    EXPECT_EQ(next_result->key(), QKeySequence(Qt::Key_F3));
    EXPECT_EQ(previous_result->key(), QKeySequence(Qt::SHIFT | Qt::Key_F3));
    EXPECT_EQ(copy_entry->key(), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_C));
    EXPECT_EQ(close_view->key(), QKeySequence(Qt::CTRL | Qt::Key_W));
    EXPECT_EQ(next_tab->key(), QKeySequence(Qt::CTRL | Qt::Key_Tab));
    EXPECT_EQ(previous_tab->key(), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Tab));

    ASSERT_EQ(tab_widget->currentIndex(), 1);
    emit next_tab->activated();
    EXPECT_EQ(tab_widget->currentIndex(), 0);
    emit previous_tab->activated();
    EXPECT_EQ(tab_widget->currentIndex(), 1);

    emit close_view->activated();
    EXPECT_EQ(tab_widget->count(), 1);
}
