#pragma once

#include <gtest/gtest.h>

#include <QSignalSpy>
#include <QTemporaryFile>
#include <QUuid>

#include "Qt-LogViewer/Controllers/LogIngestController.h"
#include "Qt-LogViewer/Models/LogEntry.h"

/**
 * @file LogIngestControllerTest.h
 * @brief Test fixture for LogIngestController.
 *
 * Covers construction/destruction, synchronous helpers (load_file_sync, read_first_log_entry),
 * queue helpers (enqueue_stream, start_next_if_idle, cancel_for_view), getters (active/pendings),
 * and end-to-end async streaming (entry_batch_parsed, progress, finished, idle) including invalid
 * and valid paths.
 */
class LogIngestControllerTest: public ::testing::Test
{
    protected:
        LogIngestControllerTest();
        ~LogIngestControllerTest() override;

        void SetUp() override;
        void TearDown() override;

        /**
         * @brief Creates a temporary log file with some dummy lines (valid for service format).
         * @return Absolute path to the temporary file (persists until tear down).
         */
        [[nodiscard]] auto make_temp_log_file() -> QString;

        /**
         * @brief Builds a platform-agnostic path that is guaranteed to not exist.
         * @return Absolute file path that does not exist at the time of call.
         */
        [[nodiscard]] auto make_nonexistent_path() const -> QString;

        LogIngestController* m_ctrl = nullptr;
        QString m_temp_log_path;
};
