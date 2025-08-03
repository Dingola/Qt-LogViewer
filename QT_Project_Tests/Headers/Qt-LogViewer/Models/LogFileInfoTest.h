#pragma once

#include <gtest/gtest.h>

#include "Qt-LogViewer/Models/LogFileInfo.h"

/**
 * @file LogFileInfoTest.h
 * @brief Test fixture for LogFileInfo.
 */
class LogFileInfoTest: public ::testing::Test
{
    protected:
        LogFileInfoTest() = default;
        ~LogFileInfoTest() override = default;

        void SetUp() override;
        void TearDown() override;
};
