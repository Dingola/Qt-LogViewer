#pragma once

#include <gtest/gtest.h>

#include <QDateTime>
#include <QUuid>

#include "Qt-LogViewer/Models/LogEntry.h"

class FilterCoordinator;
class LogHistoryService;
class LogPageCoordinator;
class LogQueryController;
class ViewRegistry;

/**
 * @file LogQueryControllerTest.h
 * @brief Declares the fixture for complete per-view query update tests.
 */

/**
 * @class LogQueryControllerTest
 * @brief Provides a view, history database, and query coordinators for each test.
 */
class LogQueryControllerTest: public ::testing::Test
{
    protected:
        /**
         * @brief Creates the isolated controller graph used by each test.
         */
        void SetUp() override;

        /**
         * @brief Removes test history and destroys the controller graph.
         */
        void TearDown() override;

        /**
         * @brief Creates a deterministic log entry.
         * @param index Sequence number included in the message and timestamp.
         * @param file_path Source file path.
         * @return Constructed log entry.
         */
        [[nodiscard]] auto create_entry(int index, const QString& file_path) const -> LogEntry;

    protected:
        LogHistoryService* m_history{nullptr};
        ViewRegistry* m_views{nullptr};
        FilterCoordinator* m_filters{nullptr};
        LogPageCoordinator* m_pages{nullptr};
        LogQueryController* m_queries{nullptr};
        QUuid m_view_id;
};
