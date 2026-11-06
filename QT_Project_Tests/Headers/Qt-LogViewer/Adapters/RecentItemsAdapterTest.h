#pragma once

#include <gtest/gtest.h>

#include "QtCommonLib/TestSupport/TestFileSystem.h"

/**
 * @file RecentItemsAdapterTest.h
 * @brief Declares tests for recent-item model synchronization.
 */
class RecentItemsAdapterTest: public ::testing::Test
{
    protected:
        RecentItemsAdapterTest() = default;
        ~RecentItemsAdapterTest() override = default;

        /** @brief Owns isolated recent files used by availability-refresh tests. */
        QtCommonLib::TestFileSystem m_file_system;
};
