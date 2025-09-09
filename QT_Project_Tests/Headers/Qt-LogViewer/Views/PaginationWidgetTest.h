#pragma once

#include <gtest/gtest.h>

#include <QSignalSpy>

#include "Qt-LogViewer/Views/Shared/PaginationWidget.h"

/**
 * @file PaginationWidgetTest.h
 * @brief Test fixture for PaginationWidget.
 */
class PaginationWidgetTest: public ::testing::Test
{
    protected:
        PaginationWidgetTest() = default;
        ~PaginationWidgetTest() override = default;

        void SetUp() override;
        void TearDown() override;

        PaginationWidget* m_widget = nullptr;
};
