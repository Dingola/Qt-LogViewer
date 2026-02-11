#pragma once

#include <gtest/gtest.h>

/**
 * @file LogEntryTest.h
 * @brief Test fixture for LogEntry.
 */
class LogEntryTest: public ::testing::Test
{
    protected:
        LogEntryTest() = default;
        ~LogEntryTest() override = default;

        void SetUp() override;
        void TearDown() override;
};
