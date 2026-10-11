#pragma once

#include <gtest/gtest.h>

#include "QtCommonLib/TestSupport/TestFileSystem.h"

/**
 * @file ApplicationStoragePathsTest.h
 * @brief Declares tests for the canonical application storage layout.
 */

/**
 * @class ApplicationStoragePathsTest
 * @brief Groups path-resolution and settings presentation tests in isolated storage.
 */
class ApplicationStoragePathsTest: public ::testing::Test
{
    protected:
        /** @brief Isolated filesystem owning settings artifacts created by UI tests. */
        QtCommonLib::TestFileSystem m_file_system;
};
