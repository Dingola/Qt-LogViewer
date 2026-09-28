#pragma once

#include <gtest/gtest.h>

#include <QString>
#include <QUuid>
#include <QVector>

#include "Qt-LogViewer/Services/LogParsingProfile.h"

class LogFilterBarWidget;
class LogLevelPieChartWidget;
class LogTabWidget;
class LogViewerController;
class PaginationWidget;
class QPlainTextEdit;
class QStackedWidget;
class QTemporaryFile;
class WorkspacePresenter;

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

        /** @brief Destroys the workspace object graph and temporary files. */
        void TearDown() override;

        /**
         * @brief Creates a runtime view, widget, and per-view presenter.
         * @param records Complete log records written to the temporary source file.
         * @return Identifier of the added runtime view.
         */
        auto add_log_view(const QVector<QString>& records) -> QUuid;

        LogParsingProfile m_profile{LogParsingProfile::create_default(
            QStringLiteral("{timestamp} {level} {message} {app_name}"),
            QStringLiteral("Workspace presenter test default"))};
        LogViewerController* m_controller{nullptr};
        LogTabWidget* m_tab_widget{nullptr};
        LogFilterBarWidget* m_filter_bar{nullptr};
        PaginationWidget* m_pagination{nullptr};
        QPlainTextEdit* m_details_text{nullptr};
        LogLevelPieChartWidget* m_level_chart{nullptr};
        QStackedWidget* m_central_stack{nullptr};
        WorkspacePresenter* m_presenter{nullptr};
        QVector<QTemporaryFile*> m_temp_files;
};
