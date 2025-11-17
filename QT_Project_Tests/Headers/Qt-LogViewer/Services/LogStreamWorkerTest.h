#pragma once

#include <gtest/gtest.h>

#include <QString>
#include <QTemporaryFile>
#include <QVector>

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

        void SetUp() override;
        void TearDown() override;

        /**
         * @brief Helper to create a QTemporaryFile with given log lines.
         * @param lines The log lines to write.
         * @return Pointer to the created QTemporaryFile (ownership transferred).
         */
        auto create_temp_file(const QVector<QString>& lines) -> QTemporaryFile*;

        // Reusable worker for non-threaded tests (created in SetUp).
        class LogStreamWorker* m_worker = nullptr;

        // Simple format used across tests for easier line generation.
        QString m_format;

        // Track created temp files to clean up deterministically.
        QVector<QTemporaryFile*> m_temp_files;
        QVector<QString> m_temp_file_names;
};
