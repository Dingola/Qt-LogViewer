#pragma once

#include <gtest/gtest.h>

#include "Qt-LogViewer/Models/LogModel.h"
#include "Qt-LogViewer/Models/LogSortFilterProxyModel.h"

/**
 * @file LogSortFilterProxyModelTest.h
 * @brief Test fixture for LogSortFilterProxyModel.
 */
class LogSortFilterProxyModelTest: public ::testing::Test
{
    protected:
        LogSortFilterProxyModelTest() = default;
        ~LogSortFilterProxyModelTest() override = default;

        void SetUp() override;
        void TearDown() override;

        LogModel* m_model = nullptr;
        LogSortFilterProxyModel* m_proxy = nullptr;
};
