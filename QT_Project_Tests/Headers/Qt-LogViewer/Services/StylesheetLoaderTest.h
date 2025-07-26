#pragma once

#include <gtest/gtest.h>

#include <QString>

#include "Qt-LogViewer/Services/StylesheetLoader.h"

/**
 * @file StylesheetLoaderTest.h
 * @brief Test fixture for StylesheetLoader.
 */
class StylesheetLoaderTest: public ::testing::Test
{
    protected:
        StylesheetLoaderTest() = default;
        ~StylesheetLoaderTest() override = default;

        void SetUp() override;
        void TearDown() override;

        /**
         * @brief Helper to create a temporary QSS file with given content.
         * @param content The QSS content to write.
         * @return The file path of the created temporary file.
         */
        [[nodiscard]] auto create_temp_qss(const QString& content) -> QString;

        StylesheetLoader* m_loader = nullptr;
};
