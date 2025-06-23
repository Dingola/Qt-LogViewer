#pragma once

#include <gtest/gtest.h>

#include "Qt-LogViewer/Models/LogFilterProxyModel.h"
#include "Qt-LogViewer/Models/LogModel.h"

/**
 * @file LogFilterProxyModelTest.h
 * @brief Test fixture for LogFilterProxyModel.
 */
class LogFilterProxyModelTest: public ::testing::Test
{
    protected:
        LogFilterProxyModelTest() = default;
        ~LogFilterProxyModelTest() override = default;

        void SetUp() override;
        void TearDown() override;

        LogModel* m_model = nullptr;
        LogFilterProxyModel* m_proxy = nullptr;
};
