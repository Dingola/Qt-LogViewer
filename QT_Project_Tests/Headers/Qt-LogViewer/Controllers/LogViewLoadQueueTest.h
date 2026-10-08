#pragma once

#include <gtest/gtest.h>

#include <QString>
#include <QUuid>

#include "Qt-LogViewer/Controllers/LogViewLoadQueue.h"
#include "Qt-LogViewer/Services/LogLoadingService.h"
#include "QtCommonLib/TestSupport/TestFileSystem.h"

/**
 * @file LogViewLoadQueueTest.h
 * @brief Test fixture for LogViewLoadQueue.
 *
 * Covers enqueue deduplication, FIFO ordering, starting behavior, active state management,
 * pending cleanup per view, and cancel semantics.
 */
class LogViewLoadQueueTest: public ::testing::Test
{
    protected:
        LogViewLoadQueueTest();
        ~LogViewLoadQueueTest() override;

        void SetUp() override;
        void TearDown() override;

        LogViewLoadQueue m_queue;
        LogLoadingService* m_loader = nullptr;

        /** @brief Isolated filesystem providing unique queue paths. */
        QtCommonLib::TestFileSystem m_file_system;

        QUuid m_view_a;
        QUuid m_view_b;
        QUuid m_view_c;
};
