#pragma once

#include <gtest/gtest.h>

#include <QString>

#include "Qt-LogViewer/Services/Settings.h"

/**
 * @file SettingsTest.h
 * @brief Test fixture for Settings.
 */
class SettingsTest: public ::testing::Test
{
    protected:
        SettingsTest() = default;
        ~SettingsTest() override = default;

        void SetUp() override;
        void TearDown() override;

        /**
         * @brief Helper to create a temporary INI file with given content.
         * @param content The INI content to write.
         * @return The file path of the created temporary file.
         */
        [[nodiscard]] QString create_temp_ini(const QString& content);

        Settings* m_settings = nullptr;
};
