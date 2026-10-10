#pragma once

#include <gtest/gtest.h>

class StartPageWidget;

/**
 * @file StartPageWidgetTest.h
 * @brief Declares regression tests for start-page actions.
 */

/**
 * @class StartPageWidgetTest
 * @brief Owns a StartPageWidget instance for each start-page action test.
 */
class StartPageWidgetTest: public ::testing::Test
{
    protected:
        /** @brief Creates the start-page widget used by the current test. */
        void SetUp() override;

        /** @brief Destroys the start-page widget after the current test. */
        void TearDown() override;

        /** @brief Widget under test. */
        StartPageWidget* m_widget{nullptr};
};
