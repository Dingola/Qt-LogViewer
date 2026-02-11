#pragma once

#include <gtest/gtest.h>

#include <QDockWidget>
#include <QToolButton>
#include <QWidget>

#include "Qt-LogViewer/Views/Shared/DockWidget.h"

/**
 * @file DockWidgetTest.h
 * @brief Test fixture for DockWidget.
 *
 * Covers:
 * - Static title-bar creation and close-button wiring (including fallback for plain QDockWidget).
 * - Getters/setters for icon colors and size, including clamping and live button updates.
 * - Emission of `closed` on close.
 * - Event-driven icon refresh path via show/hover/press/release (no-crash and integration).
 */
class DockWidgetTest: public ::testing::Test
{
    protected:
        DockWidgetTest();
        ~DockWidgetTest() override;

        void SetUp() override;
        void TearDown() override;

        /**
         * @brief Creates and installs the custom title bar on the given dock widget.
         * @param dock The dock widget to install the title bar on.
         * @return The created title-bar widget.
         */
        [[nodiscard]] auto create_and_set_title_bar(QDockWidget* dock) -> QWidget*;

        /**
         * @brief Finds the close button within a title-bar widget.
         * @param title_bar The title-bar widget returned by create_dock_title_bar.
         * @return The close button or nullptr if not found.
         */
        [[nodiscard]] auto find_close_button(QWidget* title_bar) const -> QToolButton*;

        DockWidget* m_dock = nullptr;
};
