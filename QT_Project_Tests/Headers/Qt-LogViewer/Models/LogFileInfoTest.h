#pragma once

#include <gtest/gtest.h>

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
