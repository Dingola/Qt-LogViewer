#pragma once

#include <gtest/gtest.h>

#include "Qt-LogViewer/Services/NumberFormatUtils.h"

/**
 * @file NumberFormatUtilsTest.h
 * @brief Test fixture for NumberFormatUtils.
 */
class NumberFormatUtilsTest: public ::testing::Test
{
    protected:
        NumberFormatUtilsTest() = default;
        ~NumberFormatUtilsTest() override = default;

        void SetUp() override;
        void TearDown() override;
};
