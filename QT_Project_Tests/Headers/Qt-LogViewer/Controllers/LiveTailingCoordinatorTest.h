#pragma once

#include <gtest/gtest.h>

#include <QTemporaryDir>
#include <QUuid>

#include "Qt-LogViewer/Models/LogEntry.h"
#include "Qt-LogViewer/Services/LogParsingProfile.h"

class FilterCoordinator;
class LiveTailingCoordinator;
class LogHistoryService;
class LogPageCoordinator;
class LogQueryController;
class ViewRegistry;

/**
 * @file LiveTailingCoordinatorTest.h
 * @brief Declares tests for complete live-tailing coordination.
 */

/**
 * @class LiveTailingCoordinatorTest
 * @brief Provides an isolated file, history database, view, and query graph.
 */
class LiveTailingCoordinatorTest: public ::testing::Test
{
    protected:
        /**
         * @brief Creates the isolated live-tailing graph used by each test.
         */
        void SetUp() override;

        /**
         * @brief Stops tailing and destroys the isolated graph.
         */
        void TearDown() override;

        /**
         * @brief Creates a deterministic archived entry.
         * @param index Sequence number included in timestamp and message.
         * @return Entry associated with the fixture log file.
         */
        [[nodiscard]] auto create_entry(int index) const -> LogEntry;

        /**
         * @brief Appends one complete record to the watched log file.
         * @param message Message field written to the record.
         * @param level Level field written to the record.
         */
        auto append_record(const QString& message,
                           const QString& level = QStringLiteral("INFO")) -> void;

    protected:
        QTemporaryDir m_temporary_directory;
        QString m_file_path;
        QUuid m_view_id;
        LogParsingProfile m_profile{LogParsingProfile::create_default(
            QStringLiteral("{timestamp} {level} {message} {app_name}"))};
        LogHistoryService* m_history{nullptr};
        ViewRegistry* m_views{nullptr};
        FilterCoordinator* m_filters{nullptr};
        LogPageCoordinator* m_pages{nullptr};
        LogQueryController* m_queries{nullptr};
        LiveTailingCoordinator* m_tailing{nullptr};
};
