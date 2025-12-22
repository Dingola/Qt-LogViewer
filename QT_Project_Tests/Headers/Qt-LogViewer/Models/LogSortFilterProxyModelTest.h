#pragma once

#include <gtest/gtest.h>

#include <QSet>
#include <QString>

#include "Qt-LogViewer/Models/LogModel.h"
#include "Qt-LogViewer/Models/LogSortFilterProxyModel.h"

/**
 * @file LogSortFilterProxyModelTest.h
 * @brief Test fixture for LogSortFilterProxyModel.
 *
 * Covers app/level/search filters, file visibility filters (show-only, hidden), signals,
 * and sorting behavior.
 */
class LogSortFilterProxyModelTest: public ::testing::Test
{
    protected:
        LogSortFilterProxyModelTest() = default;
        ~LogSortFilterProxyModelTest() override = default;

        void SetUp() override;
        void TearDown() override;

        /**
         * @brief Builds a small dataset with two files and mixed levels/messages/app names.
         */
        auto seed_data() -> void;

        LogModel* m_model = nullptr;
        LogSortFilterProxyModel* m_proxy = nullptr;
};
