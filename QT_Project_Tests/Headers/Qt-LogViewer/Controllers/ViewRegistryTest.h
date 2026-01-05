#pragma once

#include <gtest/gtest.h>

#include <QList>
#include <QSignalSpy>
#include <QString>
#include <QUuid>
#include <QVector>

#include "Qt-LogViewer/Controllers/LogViewContext.h"
#include "Qt-LogViewer/Controllers/ViewRegistry.h"
#include "Qt-LogViewer/Models/LogEntry.h"
#include "Qt-LogViewer/Models/LogFileInfo.h"

/**
 * @file ViewRegistryTest.h
 * @brief Test fixture for ViewRegistry.
 *
 * Covers:
 * - View lifecycle: create_view (generated/fixed id), ensure_view, remove_view and signals.
 * - Current view management: set_current_view/get_current_view and signal emission.
 * - Lookups: get_context, get_all_view_ids, get_entries, get_file_paths.
 * - Loaded files manipulation: set_loaded_files, add_loaded_file, remove_entries_by_file and signal
 * emission.
 */
class ViewRegistryTest: public ::testing::Test
{
    protected:
        ViewRegistryTest();
        ~ViewRegistryTest() override;

        void SetUp() override;
        void TearDown() override;

        /**
         * @brief Helper to append sample entries to a context.
         * @param ctx Target LogViewContext.
         * @param file_path File path to associate with entries.
         * @param count Number of entries to append.
         */
        auto append_sample_entries(LogViewContext* ctx, const QString& file_path,
                                   int count) -> void;

        ViewRegistry* m_registry = nullptr;
};
