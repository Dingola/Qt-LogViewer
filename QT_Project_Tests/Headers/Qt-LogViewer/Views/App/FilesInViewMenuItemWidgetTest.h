#pragma once

#include <gtest/gtest.h>

#include <QColor>
#include <QEvent>
#include <QLabel>
#include <QSignalSpy>
#include <QTemporaryFile>
#include <QToolButton>
#include <QWidget>

#include "Qt-LogViewer/Views/App/FilesInViewMenuItemWidget.h"

/**
 * @file FilesInViewMenuItemWidgetTest.h
 * @brief Test fixture for FilesInViewMenuItemWidget.
 *
 * Covers file path handling, label elision and tooltip, icon configuration (colors/paths/size),
 * hidden state toggle tooltip, signals emission for actions, eventFilter hover handling, and
 * reserved label pixels behavior.
 */

/**
 * @class TestableFilesInViewMenuItemWidget
 * @brief Test helper to expose `eventFilter` via a public wrapper for unit testing.
 */
class TestableFilesInViewMenuItemWidget: public FilesInViewMenuItemWidget
{
    public:
        explicit TestableFilesInViewMenuItemWidget(QWidget* parent = nullptr)
            : FilesInViewMenuItemWidget(parent)
        {}

        /**
         * @brief Public wrapper around protected `eventFilter` to allow direct testing.
         * @param watched Watched object.
         * @param event Event to filter.
         * @return The boolean returned by `eventFilter`.
         */
        [[nodiscard]] auto call_event_filter(QObject* watched, QEvent* event) -> bool
        {
            bool result = FilesInViewMenuItemWidget::eventFilter(watched, event);
            return result;
        }
};

/**
 * @class FilesInViewMenuItemWidgetTest
 * @brief GTest fixture for FilesInViewMenuItemWidget.
 */
class FilesInViewMenuItemWidgetTest: public ::testing::Test
{
    protected:
        FilesInViewMenuItemWidgetTest();
        ~FilesInViewMenuItemWidgetTest() override;

        void SetUp() override;
        void TearDown() override;

        /**
         * @brief Accesses private child controls via QObject lookup.
         * @param widget Target widget instance.
         * @return Tuple of label and buttons for assertions.
         */
        [[nodiscard]] auto get_controls(FilesInViewMenuItemWidget* widget) const
            -> std::tuple<QLabel*, QToolButton*, QToolButton*, QToolButton*>;

        TestableFilesInViewMenuItemWidget* m_widget = nullptr;
};
