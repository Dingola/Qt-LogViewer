#pragma once

#include <gtest/gtest.h>

#include <QString>

#include "Qt-LogViewer/Services/LogParsingProfile.h"

class LogImportTabPresenter;
class LogTabWidget;
class LogViewerController;
class LogViewerSettings;
class QTemporaryDir;

/**
 * @file LogImportTabPresenterTest.h
 * @brief Declares tests for the temporary import-tab workflow.
 */

/**
 * @class LogImportTabPresenterTest
 * @brief Covers cancellation, new views, existing targets, and closed targets.
 */
class LogImportTabPresenterTest: public ::testing::Test
{
    protected:
        /** @brief Creates isolated settings, import services, and a tab container. */
        void SetUp() override;

        /** @brief Destroys the isolated import workflow and temporary files. */
        void TearDown() override;

        /**
         * @brief Writes complete records to an isolated log file.
         * @param file_name File name relative to the temporary directory.
         * @param records Complete records to write.
         * @return Absolute path of the created file, or an empty path on failure.
         */
        auto create_log_file(const QString& file_name, const QStringList& records) const -> QString;

        LogParsingProfile m_profile{LogParsingProfile::create_default(
            QStringLiteral("{timestamp} {level} {message} {app_name}"),
            QStringLiteral("Import tab presenter test"))};
        QTemporaryDir* m_temp_dir = nullptr;
        LogViewerSettings* m_settings = nullptr;
        LogViewerController* m_controller = nullptr;
        LogTabWidget* m_tab_widget = nullptr;
        LogImportTabPresenter* m_presenter = nullptr;
};
