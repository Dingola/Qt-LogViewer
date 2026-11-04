#pragma once

#include <gtest/gtest.h>

#include "QtCommonLib/TestSupport/TestFileSystem.h"

/**
 * @file LocalFileStatusTest.h
 * @brief Declares focused tests for portable local file status inspection.
 */

/**
 * @class LocalFileStatusTest
 * @brief Owns isolated filesystem entries used by local file status tests.
 */
class LocalFileStatusTest: public ::testing::Test
{
    protected:
        /** @brief Isolated filesystem owning every inspected test entry. */
        QtCommonLib::TestFileSystem m_file_system;
};
