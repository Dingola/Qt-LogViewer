#pragma once

#include <gtest/gtest.h>

#include <QString>
#include <QUuid>

class LogViewPresenter;
class LogViewWidget;
class TestLogRuntime;
class QTemporaryFile;

/**
 * @file LogViewPresenterTest.h
 * @brief Declares tests for the per-view presentation binding.
 */

/**
 * @class LogViewPresenterTest
 * @brief Exercises widget initialization, routed actions, and presenter lifetime.
 */
class LogViewPresenterTest: public ::testing::Test
{
    protected:
        /** @brief Creates one controller view, widget, and bound presenter. */
        void SetUp() override;

        /** @brief Destroys the widget-owned presenter and runtime components. */
        void TearDown() override;

        TestLogRuntime* m_runtime{nullptr};
        LogViewWidget* m_widget{nullptr};
        LogViewPresenter* m_presenter{nullptr};
        QTemporaryFile* m_log_file{nullptr};
        QString m_file_path;
        QUuid m_view_id;
};
