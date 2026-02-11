#pragma once

#include <gtest/gtest.h>

#include <QMenu>
#include <QVector>

#include "Qt-LogViewer/Views/App/FilesInViewMenuItemWidget.h"
#include "Qt-LogViewer/Views/App/LogViewWidget.h"

/**
 * @file LogViewWidgetTest.h
 * @brief Test fixture for LogViewWidget.
 *
 * Covers view id management, model wiring and selection forwarding, filter widget API forwarding,
 * files-in-view menu rebuilding and action signal forwarding, and simple visibility/clear helpers.
 */
class LogViewWidgetTest: public ::testing::Test
{
    protected:
        LogViewWidgetTest();
        ~LogViewWidgetTest() override;

        void SetUp() override;
        void TearDown() override;

        /**
         * @brief Helper: locate the files-in-view menu created by the widget.
         * @return Pointer to the QMenu or nullptr if not found.
         */
        [[nodiscard]] auto find_files_menu() const -> QMenu*;

        /**
         * @brief Helper: show files menu and return created per-file row widgets.
         *        Caller must ensure the LogViewWidget remains alive while using the rows.
         *
         * @return Vector of created FilesInViewMenuItemWidget pointers (may be empty).
         */
        [[nodiscard]] auto show_and_collect_file_rows() -> QVector<FilesInViewMenuItemWidget*>;

        LogViewWidget* m_widget = nullptr;
};
