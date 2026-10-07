#pragma once

#include <gtest/gtest.h>

#include <QVector>

#include "Qt-LogViewer/Services/LogParsingProfile.h"
#include "Qt-LogViewer/TestSupport/TestFileSystem.h"

class TestLogRuntime;

/**
 * @file LogCacheReadServiceTest.h
 * @brief Declares integration tests for persistent cache-backed view queries.
 */

/**
 * @class LogCacheReadServiceTest
 * @brief Verifies cache reuse through the production import and query composition.
 */
class LogCacheReadServiceTest: public ::testing::Test
{
    protected:
        /** @brief Creates an isolated production runtime for each test. */
        auto SetUp() -> void override;

        /** @brief Stops the runtime after each test. */
        auto TearDown() -> void override;

        /**
         * @brief Creates an isolated UTF-8 log file.
         * @param records Complete source records in file order.
         * @return Absolute file path, or an empty string when creation failed.
         */
        [[nodiscard]] auto create_log_file(const QVector<QString>& records) -> QString;

        LogParsingProfile m_profile{LogParsingProfile::create_default(
            QStringLiteral("{timestamp} {level} {message} {app_name}"),
            QStringLiteral("Cache read integration"))};
        TestLogRuntime* m_runtime{nullptr};
        TestFileSystem m_file_system;
};
