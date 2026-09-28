#pragma once

#include <gtest/gtest.h>

/**
 * @file DockControllerTest.h
 * @brief Test fixture for DockController.
 *
 * Covers dock suspension and preservation of user-selected dock extents across repeated
 * horizontal, vertical and diagonal main-window resize cycles.
 */
class DockControllerTest: public ::testing::Test
{
    protected:
        DockControllerTest();
        ~DockControllerTest() override;

        /**
         * @brief Processes pending Qt events until dock layouts have settled.
         */
        static auto process_layout_events() -> void;
};
