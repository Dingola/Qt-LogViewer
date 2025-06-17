#pragma once

#include <gtest/gtest.h>

#include "Qt-LogViewer/Services/LogParser.h"

/**
 * @file LogParserTest.h
 * @brief Test fixture for LogParser.
 */
class LogParserTest: public ::testing::Test
{
    protected:
        LogParserTest() = default;
        ~LogParserTest() override = default;

        void SetUp() override;
        void TearDown() override;

        LogParser* m_parser = nullptr;
};
