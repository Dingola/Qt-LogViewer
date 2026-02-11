#pragma once

#include <gtest/gtest.h>

#include <QCheckBox>
#include <QLabel>
#include <QString>

#include "Qt-LogViewer/Views/App/LogLevelFilterItemWidget.h"

/**
 * @file LogLevelFilterItemWidgetTest.h
 * @brief Test fixture for LogLevelFilterItemWidget.
 */
class LogLevelFilterItemWidgetTest: public ::testing::Test
{
    protected:
        LogLevelFilterItemWidgetTest() = default;
        ~LogLevelFilterItemWidgetTest() override = default;

        void SetUp() override;
        void TearDown() override;

        /**
         * @brief Helper to get the checkbox widget using the current log level name.
         * @return Pointer to the QCheckBox, or nullptr.
         */
        [[nodiscard]] auto get_checkbox() const -> QCheckBox*;

        /**
         * @brief Helper to get the count label widget.
         * @return Pointer to the QLabel, or nullptr.
         */
        [[nodiscard]] auto get_count_label() const -> QLabel*;

        /**
         * @brief Helper to get the current checkbox object name.
         * @return The object name for the checkbox.
         */
        [[nodiscard]] auto get_checkbox_object_name() const -> QString;

        LogLevelFilterItemWidget* m_widget = nullptr;
};
