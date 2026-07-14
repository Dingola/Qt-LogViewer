#pragma once

#include <gtest/gtest.h>

#include "Qt-LogViewer/Services/LogHistoryService.h"

/**
 * @file LogHistoryServiceTest.h
 * @brief Declares the LogHistoryService SQLite and FTS5 test fixture.
 */
class LogHistoryServiceTest: public ::testing::Test
{
    protected:
        /**
         * @brief Creates an isolated history-service instance.
         */
        void SetUp() override;

        /**
         * @brief Destroys the isolated history-service instance.
         */
        void TearDown() override;

        /**
         * @brief Creates one deterministic test entry.
         * @param message Entry message.
         * @param file_path Source file path.
         * @return Constructed parsed log entry.
         */
        [[nodiscard]] auto create_entry(const QString& message,
                                        const QString& file_path) const -> LogEntry;

    protected:
        LogHistoryService* m_history_service{nullptr};
        QUuid m_view_id;
};
