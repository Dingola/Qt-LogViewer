#pragma once

#include <gtest/gtest.h>

#include <QString>

#include "Qt-LogViewer/Services/AppSettings.h"

/**
 * @file AppSettingsTest.h
 * @brief Test fixture for AppSettings.
 */
class AppSettingsTest: public ::testing::Test
{
    protected:
        AppSettingsTest() = default;
        ~AppSettingsTest() override = default;

        void SetUp() override;
        void TearDown() override;

        AppSettings* m_app_settings = nullptr;
};
