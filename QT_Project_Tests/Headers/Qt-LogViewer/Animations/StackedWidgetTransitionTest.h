#pragma once

#include <gtest/gtest.h>

class QStackedWidget;
class StackedWidgetTransition;

/**
 * @file StackedWidgetTransitionTest.h
 * @brief Declares tests for reusable stacked-widget page transitions.
 */

/**
 * @class StackedWidgetTransitionTest
 * @brief Verifies immediate, animated, and configurable page changes.
 */
class StackedWidgetTransitionTest: public ::testing::Test
{
    protected:
        /** @brief Creates a two-page stack and its transition helper. */
        void SetUp() override;

        /** @brief Destroys the transition and its stack. */
        void TearDown() override;

        QStackedWidget* m_stack{nullptr};
        StackedWidgetTransition* m_transition{nullptr};
};
