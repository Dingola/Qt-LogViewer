#pragma once

#include <gtest/gtest.h>

#include "Qt-LogViewer/Views/Shared/Dialog.h"

/**
 * @file DialogTest.h
 * @brief Test fixture for Dialog.
 */
class DialogTest: public ::testing::Test
{
    protected:
        DialogTest() = default;
        ~DialogTest() override = default;

        void SetUp() override;
        void TearDown() override;

        Dialog* m_dialog = nullptr;
};
