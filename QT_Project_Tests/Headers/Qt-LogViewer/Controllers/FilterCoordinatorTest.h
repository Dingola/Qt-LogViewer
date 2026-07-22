#pragma once

#include <gtest/gtest.h>

#include <QString>
#include <QUuid>

#include "Qt-LogViewer/Controllers/FilterCoordinator.h"
#include "Qt-LogViewer/Controllers/ViewRegistry.h"
#include "Qt-LogViewer/Models/SessionTypes.h"

/**
 * @file FilterCoordinatorTest.h
 * @brief Test fixture for FilterCoordinator.
 *
 * Covers:
 * - Storage and retrieval of per-view filter state.
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
         * @brief Returns the filter state of a view.
         * @param view_id Target view id.
         * @return Current filter state or a default state if the view does not exist.
         */
        [[nodiscard]] auto filter_state_for(const QUuid& view_id) const -> FilterState;

        ViewRegistry* m_views = nullptr;
        FilterCoordinator* m_fc = nullptr;
        QUuid m_view;
};
