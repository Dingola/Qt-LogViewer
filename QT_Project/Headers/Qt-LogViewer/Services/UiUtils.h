/**
 * @file UiUtils.h
 * @brief Utility class for SVG icon rendering and coloring.
 */

#pragma once

#include <QColor>
#include <QPixmap>
#include <QSize>
#include <QString>

/**
 * @class UiUtils
 * @brief Provides static helper methods for rendering and coloring SVG icons.
 *
 * This class offers utility functions to create colored QPixmaps from SVG files,
 * suitable for use in Qt widgets and actions.
 */
class UiUtils
{
    public:
        /**
         * @brief Renders an SVG file to a QPixmap and applies a color overlay.
         *
         * This method loads the SVG at the given path, renders it to a pixmap of the specified
         * size, and applies the given color using source-in composition.
         *
         * @param svg_path The path to the SVG resource.
         * @param color The color to apply to the SVG.
         * @param size The desired size of the resulting pixmap (default: 24x24).
         * @return A QPixmap containing the colored SVG.
         */
        [[nodiscard]] static auto colored_svg_icon(const QString& svg_path, const QColor& color,
                                                   QSize size = QSize(24, 24)) -> QPixmap;
};
