#pragma once

#include <gtest/gtest.h>

#include <QTemporaryFile>
#include <QVector>

#include "Qt-LogViewer/Services/LogParsingProfile.h"

class SessionController;
class TestLogRuntime;

/**
 * @file SessionControllerTest.h
 * @brief Declares integration tests for coordinated typed session restoration.
 */

/**
 * @class SessionControllerTest
 * @brief Exercises session restoration across view import, query state, and profile resolution.
 */
class SessionControllerTest: public ::testing::Test
{
    protected:
        /** @brief Creates the controller graph used by each restoration test. */
        void SetUp() override;

        /** @brief Destroys controllers and removes temporary files. */
        void TearDown() override;

        /**
         * @brief Creates a temporary log file containing the supplied records.
         * @param lines Records written in source order.
         * @return Created temporary file, or nullptr when creation fails.
         */
        auto create_temp_file(const QVector<QString>& lines) -> QTemporaryFile*;

        LogParsingProfile m_default_profile{LogParsingProfile::create_default(
            QStringLiteral("{timestamp} {level} {message} {app_name}"),
            QStringLiteral("Session test default"))};
        TestLogRuntime* m_runtime{nullptr};
        SessionController* m_session_controller{nullptr};
        QVector<QTemporaryFile*> m_temp_files;
};
