#pragma once

#include <gtest/gtest.h>

class QLabel;
class SnapshotTransitionAnimator;
class QWidget;

/**
 * @file SnapshotTransitionAnimatorTest.h
 * @brief Declares tests for generic snapshot-based widget transitions.
 */

/**
 * @class SnapshotTransitionAnimatorTest
 * @brief Verifies generic state changes, effects, and configuration validation.
 */
class SnapshotTransitionAnimatorTest: public ::testing::Test
{
    protected:
        /** @brief Creates a visible widget surface and its animator. */
        void SetUp() override;

        /** @brief Destroys the animator and widget surface. */
        void TearDown() override;

        QWidget* m_surface{nullptr};
        QLabel* m_label{nullptr};
        SnapshotTransitionAnimator* m_animator{nullptr};
};
