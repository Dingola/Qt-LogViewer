#pragma once

#include <gtest/gtest.h>

#include <QString>

#include "Qt-LogViewer/Services/Settings.h"
#include "QtCommonLib/TestSupport/TestFileSystem.h"

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
        [[nodiscard]] auto create_temp_ini(const QString& content) -> QString;

        Settings* m_settings = nullptr;
        /** @brief Isolated filesystem owning settings files created by each test. */
        QtCommonLib::TestFileSystem m_file_system;
};
