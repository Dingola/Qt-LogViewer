#pragma once

#include <gtest/gtest.h>

#include "Qt-LogViewer/Models/LogModel.h"

/**
 * @file LogModelTest.h
 * @brief Test fixture for LogModel.
 */
class LogModelTest: public ::testing::Test
{
    protected:
        LogModelTest() = default;
        ~LogModelTest() override = default;

        void SetUp() override;
        void TearDown() override;

        LogModel m_model;
};
