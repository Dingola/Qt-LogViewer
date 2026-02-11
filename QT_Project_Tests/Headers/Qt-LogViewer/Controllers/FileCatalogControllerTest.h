#pragma once

#include <gtest/gtest.h>

#include "Qt-LogViewer/Controllers/FileCatalogController.h"
#include "Qt-LogViewer/Controllers/LogIngestController.h"
#include "Qt-LogViewer/Models/LogFileTreeModel.h"

/**
 * @file FileCatalogControllerTest.h
 * @brief Test fixture for FileCatalogController.
 *
 * Covers:
 * - Adding a single file using ingest-parsed first entry (known format).
 * - Adding multiple files via `add_files`.
 * - Removing a file from the catalog.
 * - Fallback path when ingest is null (uses LogLoader::identify_app).
 */
class FileCatalogControllerTest: public ::testing::Test
{
    protected:
        FileCatalogControllerTest();
        ~FileCatalogControllerTest() override;

        void SetUp() override;
        void TearDown() override;

        /**
         * @brief Creates a temporary log file with content matching the service format.
         * Format: "{timestamp} {level} {message} {app_name} [{file}:{line} ({function})]"
         *
         * @param stem Optional stem to include in the filename.
         * @return Absolute path to the temporary file (persists until tear down).
         */
        [[nodiscard]] auto make_temp_log_file(const QString& stem = QStringLiteral("fc"))
            -> QString;

        /**
         * @brief Returns a unique, non-existent file path in the temp directory.
         *
         * @param stem Optional stem to include in the filename.
         * @return Absolute path to a non-existent file.
         */
        [[nodiscard]] auto make_nonexistent_path(
            const QString& stem = QStringLiteral("missing")) const -> QString;

        /**
         * @brief Returns the root row count of the tree model.
         *
         * @param model Target log file tree model.
         * @return Row count at root level.
         */
        [[nodiscard]] static auto root_row_count(LogFileTreeModel* model) -> int;

        FileCatalogController* m_ctrl = nullptr;
        LogIngestController* m_ingest = nullptr;
        QString m_temp1;
        QString m_temp2;
};
