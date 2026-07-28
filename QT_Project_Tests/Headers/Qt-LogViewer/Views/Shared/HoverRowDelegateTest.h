#pragma once

#include <gtest/gtest.h>

#include "Qt-LogViewer/Views/Shared/HoverRowDelegate.h"

/**
 * @file HoverRowDelegateTest.h
 * @brief Declares tests for search-result range calculation in HoverRowDelegate.
 */

/**
 * @class HoverRowDelegateTest
 * @brief Provides a delegate instance for text and regular-expression highlight tests.
 */
class HoverRowDelegateTest: public ::testing::Test
{
    protected:
        HoverRowDelegate m_delegate;
};
