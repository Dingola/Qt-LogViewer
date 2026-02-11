#pragma once

#include <gtest/gtest.h>

#include <QWidget>

#include "Qt-LogViewer/Views/Shared/FlowLayout.h"

/**
 * @file FlowLayoutTest.h
 * @brief Test fixture for FlowLayout.
 */
class FlowLayoutTest: public ::testing::Test
{
    protected:
        FlowLayoutTest() = default;
        ~FlowLayoutTest() override = default;

        void SetUp() override;
        void TearDown() override;

        QWidget* m_parent_widget = nullptr;
        FlowLayout* m_layout = nullptr;
};
