#pragma once

#include <gtest/gtest.h>

class FileInfoWidget;

/**
 * @file FileInfoWidgetTest.h
 * @brief Declares presentation tests for the file inspector widget.
 */

/** @class FileInfoWidgetTest @brief Exercises empty, aggregate, and selected-source states. */
class FileInfoWidgetTest: public ::testing::Test
{
    protected:
        /** @brief Creates a fresh file inspector widget. */
        void SetUp() override;

        /** @brief Destroys the file inspector widget. */
        void TearDown() override;

        FileInfoWidget* m_widget{nullptr};
};
