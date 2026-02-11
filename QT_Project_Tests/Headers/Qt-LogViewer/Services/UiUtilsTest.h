#pragma once

#include <gtest/gtest.h>

#include <QTemporaryFile>

/**
 * @file UiUtilsTest.h
 * @brief Test fixture for UiUtils.
 */
class UiUtilsTest: public ::testing::Test
{
    protected:
        UiUtilsTest() = default;
        ~UiUtilsTest() override = default;

        void SetUp() override;
        void TearDown() override;

        // Helper to create a temporary SVG file with simple content and return its path.
        auto create_temp_svg(const QByteArray& svg_content) -> QString;

        QTemporaryFile m_temp_svg;
};
