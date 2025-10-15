#pragma once

#include <gtest/gtest.h>

#include <QSet>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QWidget>

#include "Qt-LogViewer/Views/App/LogFilterBarWidget.h"
#include "Qt-LogViewer/Views/App/LogLevelFilterItemWidget.h"

/**
 * @file LogFilterBarWidgetTest.h
 * @brief Test fixture for LogFilterBarWidget.
 */
class LogFilterBarWidgetTest: public ::testing::Test
{
    protected:
        LogFilterBarWidgetTest() = default;
        ~LogFilterBarWidgetTest() override = default;

        void SetUp() override;
        void TearDown() override;

        /**
         * @brief Helper to get the contained LogFilterWidget.
         * @return Pointer to LogFilterWidget, or nullptr.
         */
        [[nodiscard]] auto get_filter_widget() const -> LogFilterWidget*;

        /**
         * @brief Helper to get the contained SearchBarWidget.
         * @return Pointer to SearchBarWidget, or nullptr.
         */
        [[nodiscard]] auto get_search_bar_widget() const -> SearchBarWidget*;

        /**
         * @brief Helper to get the log level filter item widget for a given log level.
         * @param level The log level name (case-insensitive).
         * @return Pointer to LogLevelFilterItemWidget, or nullptr.
         */
        [[nodiscard]] auto get_log_level_item(const QString& level) const
            -> LogLevelFilterItemWidget*;

        LogFilterBarWidget* m_widget = nullptr;
};
