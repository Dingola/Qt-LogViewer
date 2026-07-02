#pragma once

#include <gtest/gtest.h>

#include <QStandardItemModel>

#include "Qt-LogViewer/Models/PagingProxyModel.h"

/**
 * @file PagingProxyModelTest.h
 * @brief Test fixture for PagingProxyModel.
 *
 * Covers baseline paging behavior, signal diagnostics and performance baselines for
 * high-frequency source model updates (insert/remove/dataChanged).
 */
class PagingProxyModelTest: public ::testing::Test
{
    protected:
        PagingProxyModelTest() = default;
        ~PagingProxyModelTest() override = default;

        void SetUp() override;
        void TearDown() override;

        /**
         * @brief Seeds the source model with sequential rows in column 0.
         * @param row_count Number of rows to create.
         */
        auto seed_rows(int row_count) -> void;

        /**
         * @brief Resets source model size quickly for benchmark setup.
         * @param row_count Number of rows to create.
         */
        auto set_source_row_count(int row_count) -> void;

        QStandardItemModel* m_source = nullptr;
        PagingProxyModel* m_proxy = nullptr;
};
