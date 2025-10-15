#pragma once

#include <gtest/gtest.h>

#include <QMap>
#include <QSet>
#include <QString>
#include <QVector>
#include <QWidget>

#include "Qt-LogViewer/Views/App/LogFilterWidget.h"

/**
 * @file LogFilterWidgetTest.h
 * @brief Test fixture for LogFilterWidget.
 */
class LogFilterWidgetTest: public ::testing::Test
{
    protected:
        LogFilterWidgetTest() = default;
        ~LogFilterWidgetTest() override = default;

        void SetUp() override;
        void TearDown() override;

        /**
         * @brief Helper to get the log level filter item widget for a given log level.
         * @param level The log level name (case-insensitive).
         * @return Pointer to the LogLevelFilterItemWidget, or nullptr.
         */
        [[nodiscard]] auto get_log_level_item(const QString& level) const
            -> LogLevelFilterItemWidget*;

        LogFilterWidget* m_widget = nullptr;
};
