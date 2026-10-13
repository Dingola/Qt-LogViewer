#pragma once

#include <gtest/gtest.h>

#include <QString>
#include <QUuid>
#include <QVector>

#include "Qt-LogViewer/Services/LogParsingProfile.h"

class LogFilterBarWidget;
class LogDataStatusWidget;
class LogLevelPieChartWidget;
class LogTabWidget;
class TestLogRuntime;
class PaginationWidget;
class QPlainTextEdit;
class QStackedWidget;
class SessionController;
class WorkspacePresenter;
class EmptyStateWidget;

namespace QtCommonLib
{
class TestFileSystem;
}

/**
 * @file WorkspacePresenterTest.h
 * @brief Declares integration tests for shared workspace presentation.
 */

/**
 * @class WorkspacePresenterTest
 * @brief Exercises active tabs, shared controls, details, and start-page
 * selection.
 */
class WorkspacePresenterTest: public ::testing::Test
{
    protected:
        /** @brief Creates controller state and all shared workspace widgets. */
        void SetUp() override;

        /** @brief Destroys the workspace object graph and isolated test filesystem. */
        void TearDown() override;

        /**
         * @brief Creates a runtime view, widget, and per-view presenter.
         * @param records Complete log records written to an isolated source file.
         * @return Identifier of the added runtime view.
         */
        auto add_log_view(const QVector<QString>& records) -> QUuid;

        LogParsingProfile m_profile{LogParsingProfile::create_default(
            QStringLiteral("{timestamp} {level} {message} {app_name}"),
            QStringLiteral("Workspace presenter test default"))};
        TestLogRuntime* m_runtime{nullptr};
        LogTabWidget* m_tab_widget{nullptr};
        LogFilterBarWidget* m_filter_bar{nullptr};
        PaginationWidget* m_pagination{nullptr};
        QPlainTextEdit* m_details_text{nullptr};
        QStackedWidget* m_details_stack{nullptr};
        EmptyStateWidget* m_details_empty_state{nullptr};
        QStackedWidget* m_workspace_content_stack{nullptr};
        EmptyStateWidget* m_workspace_empty_state{nullptr};
        LogLevelPieChartWidget* m_level_chart{nullptr};
        LogDataStatusWidget* m_data_status{nullptr};
        QStackedWidget* m_central_stack{nullptr};
        SessionController* m_session_controller{nullptr};
        WorkspacePresenter* m_presenter{nullptr};
        QtCommonLib::TestFileSystem* m_file_system{nullptr};
        int m_next_file_id{0};
};
