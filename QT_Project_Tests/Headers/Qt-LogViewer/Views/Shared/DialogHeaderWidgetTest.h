#pragma once

#include <gtest/gtest.h>

#include "Qt-LogViewer/Views/Shared/DialogHeaderWidget.h"

/**
 * @file DialogHeaderWidgetTest.h
 * @brief Test fixture for DialogHeaderWidget.
 */
class DialogHeaderWidgetTest: public ::testing::Test
{
    protected:
        DialogHeaderWidgetTest() = default;
        ~DialogHeaderWidgetTest() override = default;

        void SetUp() override;
        void TearDown() override;

        DialogHeaderWidget* m_header = nullptr;
};
