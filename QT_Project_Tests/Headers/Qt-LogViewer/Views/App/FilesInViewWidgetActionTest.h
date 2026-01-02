#pragma once

#include <gtest/gtest.h>

#include <QColor>
#include <QLabel>
#include <QMenu>
#include <QSignalSpy>
#include <QToolButton>
#include <QWidgetAction>

#include "Qt-LogViewer/Views/App/FilesInViewMenuItemWidget.h"
#include "Qt-LogViewer/Views/App/FilesInViewWidgetAction.h"

/**
 * @file FilesInViewWidgetActionTest.h
 * @brief Test fixture for FilesInViewWidgetAction.
 *
 * Covers getters/setters (file path, icon paths/colors/size, hidden flag), widget creation,
 * signal forwarding, and hidden-state propagation to the embedded row widget.
 */
class FilesInViewWidgetActionTest: public ::testing::Test
{
    protected:
        FilesInViewWidgetActionTest();
        ~FilesInViewWidgetActionTest() override;

        void SetUp() override;
        void TearDown() override;

        /**
         * @brief Creates a hosted row widget via QMenu+QWidgetAction and returns the created
         * widget.
         * @param action The action to add to a temporary QMenu.
         * @return Pointer to the created embedded `FilesInViewMenuItemWidget`.
         */
        [[nodiscard]] auto create_hosted_row(FilesInViewWidgetAction* action)
            -> FilesInViewMenuItemWidget*;

        /**
         * @brief Finds inline buttons from the embedded row widget.
         * @param row The embedded `FilesInViewMenuItemWidget`.
         * @return Tuple of `(label, btn_show_only, btn_toggle_visibility, btn_remove)`.
         */
        [[nodiscard]] auto get_row_controls(FilesInViewMenuItemWidget* row)
            -> std::tuple<QLabel*, QToolButton*, QToolButton*, QToolButton*>;

        FilesInViewWidgetAction* m_action = nullptr;
};
