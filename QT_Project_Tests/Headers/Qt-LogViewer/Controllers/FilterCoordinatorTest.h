#pragma once

#include <gtest/gtest.h>

#include <QDir>
#include <QSet>
#include <QString>
#include <QUuid>

#include "Qt-LogViewer/Controllers/FilterCoordinator.h"
#include "Qt-LogViewer/Controllers/ViewRegistry.h"
#include "Qt-LogViewer/Models/LogEntry.h"
#include "Qt-LogViewer/Models/LogSortFilterProxyModel.h"

/**
 * @file FilterCoordinatorTest.h
 * @brief Test fixture for FilterCoordinator.
 *
 * Covers:
 * - Delegation of app-name, log-level, and search filters to proxy and getters roundtrip.
 * - File visibility operations: show-only, toggle, hide and reset behavior.
 * - Level counts from ViewRegistry entries and static available levels.
 * - Adjusting visibility on per-view and global file removal.
 */
class FilterCoordinatorTest: public ::testing::Test
{
    protected:
        FilterCoordinatorTest();
        ~FilterCoordinatorTest() override;

        void SetUp() override;
        void TearDown() override;

        /**
         * @brief Creates a unique temp file path (does not create the file).
         *
         * @param stem Stem to include in the file name for identification.
         */
        [[nodiscard]] auto make_temp_abs_path(const QString& stem) const -> QString;

        /**
         * @brief Helper to get the proxy model for a view id.
         *
         * @param view_id Target view id.
         * @return Pointer to LogSortFilterProxyModel if found; nullptr otherwise.
         */
        [[nodiscard]] auto proxy_for(const QUuid& view_id) const -> LogSortFilterProxyModel*;

        ViewRegistry* m_views = nullptr;
        FilterCoordinator* m_fc = nullptr;
        QUuid m_view;
};
