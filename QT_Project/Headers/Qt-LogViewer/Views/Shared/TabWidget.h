#pragma once

#include <QColor>
#include <QTabWidget>

class TabBar;

/**
 * @file TabWidget.h
 * @brief Declaration of TabWidget using TabBar and exposing QSS properties for colored close icons.
 */

/**
 * @class TabWidget
 * @brief QTabWidget using TabBar; exposes the same QSS properties for convenience.
 *
 * Set these via QSS on this widget:
 *  - qproperty-close_icon_color
 *  - qproperty-close_icon_color_hover
 *  - qproperty-close_icon_px
 *
 * Example:
 *   TabWidget {
 *       qproperty-close_icon_color: #ffffff;
 *       qproperty-close_icon_color_hover: #42a5f5;
 *       qproperty-close_icon_px: 18;
 *   }
 */
class TabWidget: public QTabWidget
{
        Q_OBJECT
        /**
         * @brief Normal (non-hover) color for the close icon. Forwarded to TabBar.
         */
        Q_PROPERTY(QColor close_icon_color READ get_close_icon_color WRITE set_close_icon_color)
        /**
         * @brief Hover/pressed color for the close icon. Forwarded to TabBar.
         */
        Q_PROPERTY(QColor close_icon_color_hover READ get_close_icon_color_hover WRITE
                       set_close_icon_color_hover)
        /**
         * @brief Square icon size (px) for the close icon. Forwarded to TabBar.
         */
        Q_PROPERTY(int close_icon_px READ get_close_icon_px WRITE set_close_icon_px)

    public:
        /**
         * @brief Constructs the tab widget and installs a TabBar.
         * @param parent The parent widget.
         */
        explicit TabWidget(QWidget* parent = nullptr);

        /**
         * @brief Gets the normal close icon color.
         * @return The color used for the close icon.
         */
        [[nodiscard]] auto get_close_icon_color() const -> QColor;

        /**
         * @brief Sets the normal close icon color.
         * @param color The new color to use.
         */
        auto set_close_icon_color(const QColor& color) -> void;

        /**
         * @brief Gets the hover/pressed close icon color.
         * @return The hover/pressed color used for the close icon.
         */
        [[nodiscard]] auto get_close_icon_color_hover() const -> QColor;

        /**
         * @brief Sets the hover/pressed close icon color.
         * @param color The new hover/pressed color to use.
         */
        auto set_close_icon_color_hover(const QColor& color) -> void;

        /**
         * @brief Gets the close icon size in pixels (square).
         * @return The icon size in pixels.
         */
        [[nodiscard]] auto get_close_icon_px() const -> int;

        /**
         * @brief Sets the close icon size in pixels (square).
         * @param px The icon size in pixels.
         */
        auto set_close_icon_px(int px) -> void;
};
