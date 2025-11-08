#pragma once

#include <QColor>
#include <QDockWidget>
#include <QToolButton>

/**
 * @file DockWidget.h
 * @brief Declaration of DockWidget providing a custom title bar with a QSS-driven colored close
 * icon.
 */

/**
 * @class DockWidget
 * @brief Dock widget base class for application dockable panels with a customizable close button
 * icon.
 *
 * Properties (QSS-settable):
 *  - close_icon_color: normal icon color (default: #666666)
 *    Usage: DockWidget { qproperty-close_icon_color: #RRGGBB; }
 *  - close_icon_color_hover: hover/pressed icon color (default: #42a5f5)
 *    Usage: DockWidget { qproperty-close_icon_color_hover: #RRGGBB; }
 *  - close_icon_px: square icon size in pixels (default: 18)
 *    Usage: DockWidget { qproperty-close_icon_px: 20; }
 *
 * The close button is created via create_dock_title_bar() and the icon updates automatically
 * when properties change or on hover/press events through an event filter.
 */
class DockWidget: public QDockWidget
{
        Q_OBJECT
        /**
         * @brief Normal (non-hover) color for the close icon.
         */
        Q_PROPERTY(QColor close_icon_color READ get_close_icon_color WRITE set_close_icon_color)
        /**
         * @brief Hover/pressed color for the close icon.
         */
        Q_PROPERTY(QColor close_icon_color_hover READ get_close_icon_color_hover WRITE
                       set_close_icon_color_hover)
        /**
         * @brief Square icon size (px) for the close icon.
         */
        Q_PROPERTY(int close_icon_px READ get_close_icon_px WRITE set_close_icon_px)

    public:
        using QDockWidget::QDockWidget;

        /**
         * @brief Destroys the DockWidget object.
         */
        ~DockWidget() override = default;

        /**
         * @brief Creates a custom title bar widget for a QDockWidget.
         * @param dock_widget The dock widget to create the title bar for.
         * @param object_name The object name for QSS styling.
         * @return Pointer to the created QWidget.
         */
        [[nodiscard]] static auto create_dock_title_bar(QDockWidget* dock_widget) -> QWidget*;

        /**
         * @brief Gets the normal (non-hover) close icon color.
         * @return The color used for the close icon.
         */
        [[nodiscard]] auto get_close_icon_color() const -> QColor;

        /**
         * @brief Sets the normal (non-hover) close icon color and refreshes the icon.
         * @param color The new color to use.
         */
        auto set_close_icon_color(const QColor& color) -> void;

        /**
         * @brief Gets the hover/pressed close icon color.
         * @return The color used when hovering/pressing the close icon.
         */
        [[nodiscard]] auto get_close_icon_color_hover() const -> QColor;

        /**
         * @brief Sets the hover/pressed close icon color and refreshes the icon.
         * @param color The new hover color to use.
         */
        auto set_close_icon_color_hover(const QColor& color) -> void;

        /**
         * @brief Gets the close icon size in pixels (square).
         * @return The icon size in pixels.
         */
        [[nodiscard]] auto get_close_icon_px() const -> int;

        /**
         * @brief Sets the close icon size in pixels (square) and refreshes the icon.
         * @param px The icon size in pixels. Values < 1 are clamped to 1.
         */
        auto set_close_icon_px(int px) -> void;

    protected:
        /**
         * @brief Handles the close event.
         * @param event The close event.
         */
        void closeEvent(QCloseEvent* event) override;

        /**
         * @brief Re-applies the icon when the widget is shown.
         * @param event The show event.
         */
        void showEvent(QShowEvent* event) override;

        /**
         * @brief Handles hover/press events on the close button.
         * @param watched The watched object (expected: the close button).
         * @param event The event to handle.
         * @return true if consumed; false otherwise (always false).
         */
        bool eventFilter(QObject* watched, QEvent* event) override;

    signals:
        /**
         * @brief Emitted when the dock widget is closed.
         */
        void closed();

    private:
        /**
         * @brief Updates the existing close button (if any) with current properties.
         */
        auto update_close_button() -> void;

        /**
         * @brief Builds a colored close QIcon for the requested color.
         * @param color The color to tint the SVG with.
         * @return The resulting QIcon.
         */
        [[nodiscard]] auto make_close_icon(const QColor& color) const -> QIcon;

    private:
        QColor m_close_icon_color = QColor("#666666");
        QColor m_close_icon_color_hover = QColor("#42a5f5");
        int m_close_icon_px = 18;
        QToolButton* m_close_button = nullptr;
};
