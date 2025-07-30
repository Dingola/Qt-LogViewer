#pragma once

#include <gtest/gtest.h>

#include <QString>

#include "Qt-LogViewer/Services/LogViewerSettings.h"

/**
 * @file AppSettingsTest.h
 * @brief Test fixture for AppSettings.
 */
class LogViewerSettingsTest: public ::testing::Test
{
    protected:
        LogViewerSettingsTest() = default;
        ~LogViewerSettingsTest() override = default;

        void SetUp() override;
        void TearDown() override;

        LogViewerSettings* m_app_settings = nullptr;
};
