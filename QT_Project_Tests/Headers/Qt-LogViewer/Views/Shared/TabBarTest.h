#pragma once

#include <gtest/gtest.h>

#include "Qt-LogViewer/Views/Shared/TabBar.h"

/**
 * @file TabBarTest.h
 * @brief Test fixture for TabBar.
 */
class TabBarTest: public ::testing::Test
{
    protected:
        TabBarTest() = default;
        ~TabBarTest() override = default;

        void SetUp() override;
        void TearDown() override;

        TabBar* m_tab_bar = nullptr;
};
