/**
 * @file UiUtilsTest.cpp
 * @brief Tests for UiUtils::colored_svg_icon (rendering, sizing, and coloring).
 */

#include "Qt-LogViewer/Services/UiUtilsTest.h"

#include <QApplication>
#include <QDir>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QTemporaryFile>

#include "Qt-LogViewer/Services/UiUtils.h"

void UiUtilsTest::SetUp()
{
    // Ensure the temporary file object is in a clean state
    if (m_temp_svg.isOpen())
    {
        m_temp_svg.close();
    }
}

void UiUtilsTest::TearDown()
{
    if (m_temp_svg.isOpen())
    {
        m_temp_svg.close();
    }
}

auto UiUtilsTest::create_temp_svg(const QByteArray& svg_content) -> QString
{
    // Create a temporary SVG file with auto-remove on destruction
    m_temp_svg.setAutoRemove(true);
    m_temp_svg.setFileTemplate(QDir::tempPath() + "/uiutils_test_XXXXXX.svg");
    const bool created = m_temp_svg.open();
    if (created)
    {
        m_temp_svg.write(svg_content);
        m_temp_svg.flush();
        m_temp_svg.close();
    }
    return m_temp_svg.fileName();
}

/**
 * @test Renders a simple SVG at the requested size and applies the given color.
 */
TEST_F(UiUtilsTest, RendersWithColorAtRequestedSize)
{
    // Simple 24x24 viewBox with a centered circle
    const QByteArray svg =
        R"(<?xml version="1.0" encoding="UTF-8"?>
<svg width="24" height="24" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg">
  <circle cx="12" cy="12" r="10" fill="#000000"/>
</svg>)";

    const QString svg_path = create_temp_svg(svg);
    ASSERT_FALSE(svg_path.isEmpty());

    const QSize target_size(32, 32);
    const QColor color("#ff1122");

    const QPixmap pm = UiUtils::colored_svg_icon(svg_path, color, target_size);
    ASSERT_FALSE(pm.isNull());
    EXPECT_EQ(pm.size(), target_size);

    // Sample the center pixel to verify color overlay applied
    const QImage img = pm.toImage();
    const QColor sampled = QColor::fromRgb(img.pixel(img.width() / 2, img.height() / 2));

    EXPECT_EQ(sampled.red(), color.red());
    EXPECT_EQ(sampled.green(), color.green());
    EXPECT_EQ(sampled.blue(), color.blue());
    EXPECT_GT(sampled.alpha(), 0);
}

/**
 * @test Uses default size when not provided and applies color overlay.
 */
TEST_F(UiUtilsTest, UsesDefaultSizeAndColorsImage)
{
    const QByteArray svg =
        R"(<?xml version="1.0" encoding="UTF-8"?>
<svg width="24" height="24" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg">
  <rect x="2" y="2" width="20" height="20" fill="#000000"/>
</svg>)";

    const QString svg_path = create_temp_svg(svg);
    ASSERT_FALSE(svg_path.isEmpty());

    const QColor color("#42a5f5");                                  // material blue
    const QPixmap pm = UiUtils::colored_svg_icon(svg_path, color);  // default size 24x24

    ASSERT_FALSE(pm.isNull());
    EXPECT_EQ(pm.size(), QSize(24, 24));

    const QImage img = pm.toImage();
    const QColor sampled = QColor::fromRgb(img.pixel(img.width() / 2, img.height() / 2));

    EXPECT_EQ(sampled.red(), color.red());
    EXPECT_EQ(sampled.green(), color.green());
    EXPECT_EQ(sampled.blue(), color.blue());
    EXPECT_GT(sampled.alpha(), 0);
}

/**
 * @test Gracefully handles invalid SVG path by returning a pixmap of requested size.
 */
TEST_F(UiUtilsTest, InvalidPathReturnsPixmapOfRequestedSize)
{
    const QSize target_size(24, 24);
    const QPixmap pm =
        UiUtils::colored_svg_icon(":/invalid/path.svg", QColor("#00ff00"), target_size);

    ASSERT_FALSE(pm.isNull());
    EXPECT_EQ(pm.size(), target_size);
}
