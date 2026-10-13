#include "Qt-LogViewer/Views/App/StartPageWidgetTest.h"

#include <QPushButton>
#include <QSignalSpy>

#include "Qt-LogViewer/Views/App/StartPageWidget.h"

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
