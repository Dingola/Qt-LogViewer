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
