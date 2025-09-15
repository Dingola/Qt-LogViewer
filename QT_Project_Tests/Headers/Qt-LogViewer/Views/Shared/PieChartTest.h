#pragma once

#include <gtest/gtest.h>

#include <QColor>
#include <QMap>
#include <QString>

#include "Qt-LogViewer/Views/Shared/PieChart.h"

/**
 * @file PieChartTest.h
 * @brief Test fixture for PieChart.
 */
class PieChartTest: public ::testing::Test
{
    protected:
        PieChartTest() = default;
        ~PieChartTest() override = default;

        void SetUp() override;
        void TearDown() override;

        PieChart* m_chart = nullptr;
};
