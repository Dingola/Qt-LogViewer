#pragma once

#include <gtest/gtest.h>

#include <QTemporaryFile>
#include <QVector>

#include "Qt-LogViewer/Services/LogParsingProfile.h"

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

        /** @brief Stops the runtime graph and removes temporary files. */
        void TearDown() override;

        /**
         * @brief Creates a persistent temporary log file.
         * @param records Complete records written in source order.
         * @return Created file, or nullptr when the file could not be opened.
         */
        [[nodiscard]] auto create_log_file(const QVector<QString>& records) -> QTemporaryFile*;

        LogParsingProfile m_default_profile{LogParsingProfile::create_default(
            QStringLiteral("{timestamp} {level} {message} {app_name}"),
            QStringLiteral("Workflow integration default"))};
        TestLogRuntime* m_runtime{nullptr};
        QVector<QTemporaryFile*> m_temp_files;
};
