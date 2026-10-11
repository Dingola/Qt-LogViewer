#pragma once

#include <gtest/gtest.h>

class LogDataStatusWidget;

/**
 * @file LogDataStatusWidgetTest.h
 * @brief Declares tests for the compact log data status widget.
 */

/**
 * @class LogDataStatusWidgetTest
 * @brief Exercises compact values, cache states, and inactive-view presentation.
 */
class LogDataStatusWidgetTest: public ::testing::Test
{
    protected:
        /** @brief Creates a fresh data status widget. */
        void SetUp() override;

        /** @brief Destroys the data status widget. */
        void TearDown() override;

        LogDataStatusWidget* m_widget{nullptr};
};
