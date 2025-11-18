#pragma once

#include <gtest/gtest.h>

#include <QStandardItemModel>
#include <QVector>

/**
 * @file TableViewTest.h
 * @brief Test fixture for TableView (hover color and hover index signaling).
 */
class TableViewTest: public ::testing::Test
{
    protected:
        TableViewTest() = default;
        ~TableViewTest() override = default;

        /**
         * @brief Creates and populates a simple model (rows x cols) with text items.
         * @param rows Number of rows.
         * @param cols Number of columns.
         * @return Pointer to the created model (owned by the test fixture).
         */
        auto create_model(int rows, int cols) -> QStandardItemModel*;

        void SetUp() override;
        void TearDown() override;

        class TableView* m_view = nullptr;
        QStandardItemModel* m_model = nullptr;
};
