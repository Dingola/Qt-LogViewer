#pragma once

#include <gtest/gtest.h>

#include "QtCommonLib/TestSupport/TestFileSystem.h"

/**
 * @file LogViewerDropValidatorTest.h
 * @brief Declares focused tests for drag-and-drop file validation.
 */

/**
 * @class LogViewerDropValidatorTest
 * @brief Owns isolated files used to validate portable drop behavior.
 */
class LogViewerDropValidatorTest: public ::testing::Test
{
    protected:
        /** @brief Isolated filesystem owning every candidate drop file. */
        QtCommonLib::TestFileSystem m_file_system;
};
