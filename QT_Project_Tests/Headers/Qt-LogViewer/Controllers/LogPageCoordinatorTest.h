#pragma once

#include <gtest/gtest.h>

#include <QTemporaryDir>
#include <QUuid>

#include "Qt-LogViewer/Models/LogEntry.h"

class LogHistoryService;
class LogPageCoordinator;
class ViewRegistry;

/**
 * @file LogPageCoordinatorTest.h
 * @brief Test fixture for LogPageCoordinator.
 */
class LogPageCoordinatorTest: public ::testing::Test
{
    protected:
        /** @brief Creates the isolated page-coordinator graph used by each test. */
        void SetUp() override;
        /** @brief Destroys the page-coordinator graph and its temporary database. */
        void TearDown() override;

        /**
         * @brief Creates one deterministic test entry.
         * @param message Entry message.
         * @param timestamp Timestamp assigned to the entry.
         * @return Constructed log entry.
         */
        [[nodiscard]] auto create_entry(const QString& message,
                                        const QDateTime& timestamp) const -> LogEntry;

    protected:
        /** @brief Isolated directory containing the history database for one test. */
        QTemporaryDir m_temporary_directory;
        LogHistoryService* m_history_service{nullptr};
        ViewRegistry* m_views{nullptr};
        LogPageCoordinator* m_coordinator{nullptr};
        QUuid m_view_id;
};
