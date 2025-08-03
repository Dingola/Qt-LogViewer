#pragma once

#include <gtest/gtest.h>

#include "Qt-LogViewer/Models/LogFileInfo.h"
#include "Qt-LogViewer/Models/LogFileTreeModel.h"

/**
 * @file LogFileTreeModelTest.h
 * @brief Test fixture for LogFileTreeModel.
 */
class LogFileTreeModelTest: public ::testing::Test
{
    protected:
        LogFileTreeModelTest() = default;
        ~LogFileTreeModelTest() override = default;

        void SetUp() override;
        void TearDown() override;

        LogFileTreeModel* m_model = nullptr;
};
