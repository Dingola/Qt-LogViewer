#pragma once

#include <gtest/gtest.h>

#include <QTemporaryDir>

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
         * @param level Entry log level.
         * @param app_name Entry application name.
         * @return Constructed parsed log entry.
         */
        [[nodiscard]] auto create_entry(
            const QString& message, const QString& file_path,
            const QString& level = QStringLiteral("INFO"),
            const QString& app_name = QStringLiteral("HistoryTestApp")) const -> LogEntry;

        /**
         * @brief Creates a deterministic batch with one shared timestamp.
         * @param entry_count Number of entries to create.
         * @return Generated entries ordered by their record number.
         */
        [[nodiscard]] auto create_large_entry_batch(qsizetype entry_count) const
            -> QVector<LogEntry>;

        /**
         * @brief Verifies one exact message search against the archived view.
         * @param message Expected unique message.
         */
        auto expect_single_message_result(const QString& message) const -> void;

    protected:
        /** @brief Isolated directory containing the history database for one test. */
        QTemporaryDir m_temporary_directory;
        /** @brief History service constructed against the isolated test database. */
        LogHistoryService* m_history_service{nullptr};
        /** @brief Unique view identifier used to scope entries created by one test. */
        QUuid m_view_id;
};
