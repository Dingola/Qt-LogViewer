#pragma once

#include <gtest/gtest.h>

#include <QDir>
#include <QFile>
#include <QPair>
#include <QString>
#include <QTemporaryFile>
#include <QUuid>

#include "Qt-LogViewer/Controllers/LogViewLoadQueue.h"
#include "Qt-LogViewer/Services/LogLoadingService.h"

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

        /**
         * @brief Builds a platform-agnostic path that is guaranteed to not exist.
         * @return Absolute file path that does not exist at the time of call.
         */
        [[nodiscard]] auto make_nonexistent_path() const -> QString;

        LogViewLoadQueue m_queue;
        LogLoadingService* m_loader = nullptr;

        QUuid m_view_a;
        QUuid m_view_b;
        QUuid m_view_c;
};
