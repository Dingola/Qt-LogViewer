#pragma once

#include <gtest/gtest.h>

#include "Qt-LogViewer/Services/LogLoader.h"

/**
 * @file LogLoaderTest.h
 * @brief Test fixture for LogLoader.
 */
class LogLoaderTest: public ::testing::Test
{
    protected:
        LogLoaderTest() = default;
        ~LogLoaderTest() override = default;

        void SetUp() override;
        void TearDown() override;

        LogLoader* m_loader = nullptr;
};
