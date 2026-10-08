#pragma once

#include <gtest/gtest.h>

#include <QVector>

#include "Qt-LogViewer/Services/LogParsingProfile.h"
#include "QtCommonLib/TestSupport/TestFileSystem.h"

class TestLogRuntime;

/**
 * @file LogWorkflowIntegrationTest.h
 * @brief Declares facade-free integration tests for complete log workflows.
 */

/**
 * @class LogWorkflowIntegrationTest
 * @brief Verifies collaboration between import, query, tailing, and lifecycle components.
 */
class LogWorkflowIntegrationTest: public ::testing::Test
{
    protected:
        /** @brief Creates the focused runtime graph used by each workflow test. */
        void SetUp() override;

        /** @brief Stops the runtime graph after each test. */
        void TearDown() override;

        /**
         * @brief Creates an isolated UTF-8 log file.
         * @param records Complete records written in source order.
         * @return Absolute file path, or an empty string when creation failed.
         */
        [[nodiscard]] auto create_log_file(const QVector<QString>& records) -> QString;

        LogParsingProfile m_default_profile{LogParsingProfile::create_default(
            QStringLiteral("{timestamp} {level} {message} {app_name}"),
            QStringLiteral("Workflow integration default"))};
        TestLogRuntime* m_runtime{nullptr};
        QtCommonLib::TestFileSystem m_file_system;
};
