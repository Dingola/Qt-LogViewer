#pragma once

#include <gtest/gtest.h>

#include "Qt-LogViewer/Controllers/LogIngestController.h"
#include "Qt-LogViewer/TestSupport/TestFileSystem.h"

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

        LogIngestController* m_ctrl = nullptr;
        QString m_temp_log_path;

        /** @brief Isolated filesystem owning the fixture's paths and files. */
        TestFileSystem m_file_system;
};
