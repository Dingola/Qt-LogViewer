#pragma once

#include <gtest/gtest.h>

#include <QTabWidget>

#include "Qt-LogViewer/Views/Shared/TabWidget.h"

/**
 * @file TabWidgetTest.h
 * @brief Test fixture for TabWidget.
 */
class TabWidgetTest: public ::testing::Test
{
    protected:
        TabWidgetTest() = default;
        ~TabWidgetTest() override = default;

        void SetUp() override;
        void TearDown() override;

        TabWidget* m_tab_widget = nullptr;
};
