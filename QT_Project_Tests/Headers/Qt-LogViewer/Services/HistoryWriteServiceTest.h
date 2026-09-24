#pragma once

#include <gtest/gtest.h>

#include <QTemporaryDir>
#include <QUuid>

#include "Qt-LogViewer/Models/LogEntry.h"

/**
 * @file HistoryWriteServiceTest.h
 * @brief Declares tests for ordered background history writes and shutdown.
 */

/**
 * @class HistoryWriteServiceTest
 * @brief Provides an isolated SQLite path and deterministic log entries.
 */
class HistoryWriteServiceTest: public ::testing::Test
{
    protected:
        /**
         * @brief Creates a deterministic log entry.
         * @param index Sequence number included in the timestamp and message.
         * @param file_path Source file path stored in the entry.
         * @return Constructed log entry.
         */
        [[nodiscard]] auto create_entry(int index, const QString& file_path) const -> LogEntry;

        /**
         * @brief Returns the isolated SQLite database path for this test.
         * @return Absolute path below the temporary directory.
         */
        [[nodiscard]] auto get_database_path() const -> QString;

    protected:
        QTemporaryDir m_temporary_directory;
};
