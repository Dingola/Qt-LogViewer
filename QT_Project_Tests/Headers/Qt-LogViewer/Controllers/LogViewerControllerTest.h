#pragma once

#include <gtest/gtest.h>

#include "Qt-LogViewer/Controllers/LogViewerController.h"

/**
 * @file LogViewerControllerTest.h
 * @brief Test fixture for LogViewerController.
 */
class LogViewerControllerTest: public ::testing::Test
{
    protected:
        LogViewerControllerTest() = default;
        ~LogViewerControllerTest() override = default;

        void SetUp() override;
        void TearDown() override;

        LogViewerController* m_controller = nullptr;
};
