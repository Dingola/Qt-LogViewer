#pragma once

#include <gtest/gtest.h>

#include "Qt-LogViewer/Models/LogFileTreeItem.h"

/**
 * @file LogFileTreeItemTest.h
 * @brief Test fixture for LogFileTreeItem.
 */
class LogFileTreeItemTest: public ::testing::Test
{
    protected:
        LogFileTreeItemTest() = default;
        ~LogFileTreeItemTest() override = default;

        void SetUp() override;
        void TearDown() override;
};
