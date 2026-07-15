#pragma once

#include <gtest/gtest.h>

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
        void SetUp() override;
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
        LogHistoryService* m_history_service{nullptr};
        ViewRegistry* m_views{nullptr};
        LogPageCoordinator* m_coordinator{nullptr};
        QUuid m_view_id;
};
