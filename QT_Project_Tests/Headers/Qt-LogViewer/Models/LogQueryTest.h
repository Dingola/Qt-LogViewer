#pragma once

#include <gtest/gtest.h>

/**
 * @file LogQueryTest.h
 * @brief Test fixture for LogQuery.
 */
class LogQueryTest: public ::testing::Test
{
    protected:
        LogQueryTest() = default;
        ~LogQueryTest() override = default;

        void SetUp() override {}
        void TearDown() override {}
};
