#pragma once

#include <gtest/gtest.h>

#include <QString>
#include <QVector>

#include "Qt-LogViewer/TestSupport/TestFileSystem.h"

/**
 * @file LogStreamWorkerTest.h
 * @brief Test fixture for LogStreamWorker.
 *
 * Provides helpers to create temporary log files and a reusable worker instance.
 */
class LogStreamWorkerTest: public ::testing::Test
{
    protected:
        LogStreamWorkerTest() = default;
        ~LogStreamWorkerTest() override = default;

        /** @brief Creates the reusable worker before each test. */
        void SetUp() override;

        /** @brief Destroys the reusable worker after each test. */
        void TearDown() override;

        /**
         * @brief Creates an isolated UTF-8 log file with the supplied lines.
         * @param lines Log lines written in source order.
         * @return Absolute file path, or an empty string when creation failed.
         */
        [[nodiscard]] auto create_temp_file(const QVector<QString>& lines) -> QString;

        /** @brief Reusable worker for non-threaded tests. */
        class LogStreamWorker* m_worker = nullptr;

        /** @brief Simple parsing format shared by the tests. */
        QString m_format;

        /** @brief Isolated filesystem owning all files created by the fixture. */
        TestFileSystem m_file_system;
};
