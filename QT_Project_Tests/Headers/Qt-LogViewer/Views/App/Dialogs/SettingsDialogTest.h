#pragma once

#include <gtest/gtest.h>

#include "QtCommonLib/TestSupport/TestFileSystem.h"

class LogViewerSettings;
class SettingsDialog;

/**
 * @file SettingsDialogTest.h
 * @brief Declares focused tests for the application settings dialog.
 */

/**
 * @class SettingsDialogTest
 * @brief Owns an isolated settings dialog for each presentation test.
 */
class SettingsDialogTest: public ::testing::Test
{
    protected:
        /** @brief Creates an isolated settings service and dialog for the current test. */
        void SetUp() override;

        /** @brief Destroys the dialog and settings service after the current test. */
        void TearDown() override;

        /** @brief Isolated filesystem owning the settings file used by the fixture. */
        QtCommonLib::TestFileSystem m_file_system;

        /** @brief Settings service used by the dialog under test. */
        LogViewerSettings* m_settings{nullptr};

        /** @brief Settings dialog under test. */
        SettingsDialog* m_dialog{nullptr};
};
