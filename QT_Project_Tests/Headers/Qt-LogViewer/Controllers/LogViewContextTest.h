#pragma once

#include <gtest/gtest.h>

#include <QString>
#include <QVector>

#include "Qt-LogViewer/Controllers/LogViewContext.h"

/**
 * @file LogViewContextTest.h
 * @brief Test fixture for LogViewContext.
 *
 * Covers model/proxy wiring, entry append/remove, and loaded-files state management.
 */
class LogViewContextTest: public ::testing::Test
{
    protected:
        LogViewContextTest();
        ~LogViewContextTest() override;

        void SetUp() override;
        void TearDown() override;

        /**
         * @brief Helper to build a few LogEntry rows for a given file path and app name.
         * @param file_path Absolute file path.
         * @param app_name Application name.
         * @return Entries vector.
         */
        auto make_entries(const QString& file_path, const QString& app_name) -> QVector<LogEntry>;

        LogViewContext* m_ctx = nullptr;
};
