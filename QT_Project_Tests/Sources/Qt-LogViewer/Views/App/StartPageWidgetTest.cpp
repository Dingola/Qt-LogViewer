#include "Qt-LogViewer/Views/App/StartPageWidgetTest.h"

#include <QLabel>
#include <QListView>
#include <QPixmap>
#include <QPushButton>
#include <QSignalSpy>
#include <QStackedWidget>

#include "Qt-LogViewer/Models/RecentItemsModel.h"
#include "Qt-LogViewer/Models/RecentListSchema.h"
#include "Qt-LogViewer/Views/App/StartPageWidget.h"
#include "Qt-LogViewer/Views/Shared/EmptyStateWidget.h"

/**
 * @file StartPageWidgetTest.cpp
 * @brief Contains regression tests for start-page session actions.
 */

/** @brief Creates the start-page widget used by the current test. */
void StartPageWidgetTest::SetUp()
{
    m_widget = new StartPageWidget();
}

/** @brief Destroys the start-page widget after the current test. */
void StartPageWidgetTest::TearDown()
{
    delete m_widget;
    m_widget = nullptr;
}

/**
 * @test Verifies that the welcome heading presents the application mascot at a visible size.
 */
TEST_F(StartPageWidgetTest, PresentsKoalaBrandMarkWithWelcomeHeading)
{
    const auto* brand_icon_label = m_widget->findChild<QLabel*>(QStringLiteral("brandIconLabel"));

    ASSERT_NE(brand_icon_label, nullptr);
    const QPixmap brand_pixmap = brand_icon_label->pixmap();
    EXPECT_FALSE(brand_pixmap.isNull());
    EXPECT_EQ(brand_pixmap.size(), QSize(40, 40));
    EXPECT_EQ(brand_icon_label->alignment(), Qt::AlignCenter);
    EXPECT_FALSE(brand_icon_label->accessibleName().isEmpty());
}

/**
 * @test Verifies that Open Session requests the file dialog without requiring a recent selection.
 */
TEST_F(StartPageWidgetTest, OpenSessionDoesNotRequireRecentSelection)
{
    QSignalSpy open_session_spy(m_widget, &StartPageWidget::open_session_requested);
    QPushButton* open_session_button =
        m_widget->findChild<QPushButton*>(QStringLiteral("openSessionButton"));
    ASSERT_NE(open_session_button, nullptr);

    open_session_button->click();

    EXPECT_EQ(open_session_spy.count(), 1);
}

/** @test Verifies that missing recent data is explained and offers relevant
 * actions. */
TEST_F(StartPageWidgetTest, PresentsEmptyRecentLists)
{
    const auto* files_stack =
        m_widget->findChild<QStackedWidget*>(QStringLiteral("recentFilesStack"));
    const auto* sessions_stack =
        m_widget->findChild<QStackedWidget*>(QStringLiteral("recentSessionsStack"));
    const auto* files_empty =
        m_widget->findChild<EmptyStateWidget*>(QStringLiteral("recentFilesEmptyState"));
    const auto* sessions_empty =
        m_widget->findChild<EmptyStateWidget*>(QStringLiteral("recentSessionsEmptyState"));

    ASSERT_NE(files_stack, nullptr);
    ASSERT_NE(sessions_stack, nullptr);
    ASSERT_NE(files_empty, nullptr);
    ASSERT_NE(sessions_empty, nullptr);
    EXPECT_EQ(files_stack->currentWidget(), files_empty);
    EXPECT_EQ(sessions_stack->currentWidget(), sessions_empty);
    const QList<QPushButton*> file_actions = files_empty->findChildren<QPushButton*>();
    const QList<QPushButton*> session_actions = sessions_empty->findChildren<QPushButton*>();
    ASSERT_EQ(file_actions.size(), 1);
    ASSERT_EQ(session_actions.size(), 1);
    EXPECT_EQ(file_actions.constFirst()->property("actionRole").toString(),
              QStringLiteral("primary"));
    EXPECT_EQ(session_actions.constFirst()->property("actionRole").toString(),
              QStringLiteral("primary"));
}

/**
 * @test Verifies that start-page actions expose their semantic visual hierarchy.
 */
TEST_F(StartPageWidgetTest, ExposesSemanticActionHierarchy)
{
    const auto* open_file_button =
        m_widget->findChild<QPushButton*>(QStringLiteral("openFileButton"));
    const auto* open_session_button =
        m_widget->findChild<QPushButton*>(QStringLiteral("openSessionButton"));
    const auto* reopen_session_button =
        m_widget->findChild<QPushButton*>(QStringLiteral("reopenSessionButton"));
    const auto* open_selected_file_button =
        m_widget->findChild<QPushButton*>(QStringLiteral("openSelectedFileButton"));
    const auto* clear_recent_files_button =
        m_widget->findChild<QPushButton*>(QStringLiteral("clearRecentFilesButton"));
    const auto* open_selected_session_button =
        m_widget->findChild<QPushButton*>(QStringLiteral("openSelectedSessionButton"));
    const auto* delete_session_button =
        m_widget->findChild<QPushButton*>(QStringLiteral("deleteSessionButton"));

    ASSERT_NE(open_file_button, nullptr);
    ASSERT_NE(open_session_button, nullptr);
    ASSERT_NE(reopen_session_button, nullptr);
    ASSERT_NE(open_selected_file_button, nullptr);
    ASSERT_NE(clear_recent_files_button, nullptr);
    ASSERT_NE(open_selected_session_button, nullptr);
    ASSERT_NE(delete_session_button, nullptr);
    EXPECT_EQ(open_file_button->property("actionRole").toString(), QStringLiteral("primary"));
    EXPECT_EQ(open_session_button->property("actionRole").toString(), QStringLiteral("secondary"));
    EXPECT_EQ(reopen_session_button->property("actionRole").toString(), QStringLiteral("ghost"));
    EXPECT_EQ(open_selected_file_button->property("actionRole").toString(),
              QStringLiteral("primary"));
    EXPECT_EQ(clear_recent_files_button->property("actionRole").toString(),
              QStringLiteral("danger"));
    EXPECT_EQ(open_selected_session_button->property("actionRole").toString(),
              QStringLiteral("primary"));
    EXPECT_EQ(delete_session_button->property("actionRole").toString(), QStringLiteral("danger"));
}

/** @test Keeps an unavailable recent file visible and prevents primary activation. */
TEST_F(StartPageWidgetTest, PresentsUnavailableRecentFileWithoutOpeningIt)
{
    RecentItemsModel model(RecentListSchemas::make_recent_files_schema());
    QHash<int, QVariant> row;
    row.insert(to_role_id(RecentFileRole::FileName), QStringLiteral("missing.log"));
    row.insert(to_role_id(RecentFileRole::FilePath), QStringLiteral("C:/logs/missing.log"));
    row.insert(to_role_id(RecentFileRole::LastOpened), QDateTime::currentDateTime());
    row.insert(to_role_id(RecentFileRole::StatusText), QStringLiteral("File not found"));
    row.insert(to_role_id(RecentFileRole::Enabled), false);
    row.insert(Qt::ToolTipRole, QStringLiteral("File not found: C:/logs/missing.log"));
    row.insert(Qt::AccessibleDescriptionRole,
               QStringLiteral("File not found: C:/logs/missing.log"));
    model.set_rows({row});

    m_widget->set_recent_files_model(&model);

    auto* recent_files_view = m_widget->findChild<QListView*>(QStringLiteral("recentFilesView"));
    auto* open_selected_button =
        m_widget->findChild<QPushButton*>(QStringLiteral("openSelectedFileButton"));
    QSignalSpy open_spy(m_widget, &StartPageWidget::open_recent_file_requested);
    ASSERT_NE(recent_files_view, nullptr);
    ASSERT_NE(open_selected_button, nullptr);

    const QModelIndex index = model.index(0, 0);
    EXPECT_FALSE(index.flags().testFlag(Qt::ItemIsEnabled));
    EXPECT_FALSE(open_selected_button->isEnabled());
    EXPECT_EQ(index.data(Qt::AccessibleDescriptionRole), index.data(Qt::ToolTipRole));

    QMetaObject::invokeMethod(recent_files_view, "doubleClicked", Qt::DirectConnection,
                              Q_ARG(QModelIndex, index));
    EXPECT_EQ(open_spy.count(), 0);
}
