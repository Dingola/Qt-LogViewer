/**
 * @file UiUtils.cpp
 * @brief Implementation of the UiUtils utility class.
 */

#include "Qt-LogViewer/Services/UiUtils.h"

#include <QPainter>
#include <QSvgRenderer>

/**
 * @brief Renders an SVG file to a QPixmap and applies a color overlay.
 *
 * This method loads the SVG at the given path, renders it to a pixmap of the specified size,
 * and applies the given color using source-in composition.
 *
 * @param svg_path The path to the SVG resource.
 * @param color The color to apply to the SVG.
 * @param size The desired size of the resulting pixmap.
 * @return A QPixmap containing the colored SVG.
 */
auto UiUtils::colored_svg_icon(const QString& svg_path, const QColor& color, QSize size) -> QPixmap
{
    QSvgRenderer renderer(svg_path);
    QPixmap pixmap(size);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    renderer.render(&painter);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(pixmap.rect(), color);
    painter.end();
    return pixmap;
}
