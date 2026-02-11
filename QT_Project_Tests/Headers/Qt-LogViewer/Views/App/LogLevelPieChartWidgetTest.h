#pragma once

#include <gtest/gtest.h>

#include <QLabel>
#include <QWidget>

#include "Qt-LogViewer/Views/App/LogLevelPieChartWidget.h"

/**
 * @file LogLevelPieChartWidgetTest.h
 * @brief Test fixture for LogLevelPieChartWidget.
 */
class LogLevelPieChartWidgetTest: public ::testing::Test
{
    protected:
        LogLevelPieChartWidgetTest() = default;
        ~LogLevelPieChartWidgetTest() override = default;

        void SetUp() override;
        void TearDown() override;

        /**
         * @brief Helper to get the percent label for a log level.
         * @param level The log level name (e.g., "trace").
         * @return Pointer to the QLabel showing the percent, or nullptr.
         */
        [[nodiscard]] auto get_percent_label(const QString& level) const -> QLabel*;

        /**
         * @brief Helper to get the row widget for a log level.
         * @param level The log level name (e.g., "trace").
         * @return Pointer to the QWidget for the row, or nullptr.
         */
        [[nodiscard]] auto get_row_widget(const QString& level) const -> QWidget*;

        /**
         * @brief Helper to get the name label for a log level.
         * @param level The log level name (e.g., "trace").
         * @return Pointer to the QLabel showing the name, or nullptr.
         */
        [[nodiscard]] auto get_name_label(const QString& level) const -> QLabel*;

        /**
         * @brief Helper to get the color label for a log level.
         * @param level The log level name (e.g., "trace").
         * @return Pointer to the QLabel showing the color, or nullptr.
         */
        [[nodiscard]] auto get_color_label(const QString& level) const -> QLabel*;

        LogLevelPieChartWidget* m_widget = nullptr;
};
