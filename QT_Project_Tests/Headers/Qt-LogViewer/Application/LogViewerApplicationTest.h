#pragma once

#include <gtest/gtest.h>

#include <QString>

/**
 * @file LogViewerApplicationTest.h
 * @brief Declares lifecycle tests for the Qt-LogViewer application composition.
 */
class LogViewerApplicationTest: public ::testing::Test
{
    protected:
        LogViewerApplicationTest() = default;
        ~LogViewerApplicationTest() override = default;

        /**
         * @brief Captures global application styling before the complete UI applies its theme.
         */
        void SetUp() override;

        /**
         * @brief Restores global application styling for subsequently executed widget tests.
         */
        void TearDown() override;

    private:
        QString m_original_stylesheet;
};
