#pragma once

#include <QColor>
#include <QTabBar>

/**
 * @file TabBar.h
 * @brief Declaration of TabBar providing QSS-driven colored close icons for tabs.
 */

/**
 * @class TabBar
 * @brief QTabBar that colors and sizes the close button icon via QSS-configurable properties.
 *
 * Properties:
 *  - close_icon_color: normal icon color (default: #666666)
 *  - close_icon_color_hover: hover/pressed icon color (default: #42a5f5)
 *  - close_icon_px: square icon size in pixels (default: 18)
 *
 * The bar installs an event filter on generated close buttons and updates their icon
 * on hover/press. Icons are rebuilt whenever tabs are inserted, the bar is shown,
 * or a property changes.
 */
class TabBar: public QTabBar
{
        Q_OBJECT
        /**
         * @brief Normal (non-hover) color for the close icon.
         *
         * Can be set via QSS:
         *   TabBar { qproperty-close_icon_color: #RRGGBB; }
         */
        Q_PROPERTY(QColor close_icon_color READ get_close_icon_color WRITE set_close_icon_color)
        /**
         * @brief Hover/pressed color for the close icon.
         *
         * Can be set via QSS:
         *   TabBar { qproperty-close_icon_color_hover: #RRGGBB; }
         */
        Q_PROPERTY(QColor close_icon_color_hover READ get_close_icon_color_hover WRITE
                       set_close_icon_color_hover)
        /**
         * @brief Square icon size (px) for the close icon.
         *
         * Can be set via QSS:
         *   TabBar { qproperty-close_icon_px: 18; }
         */
        Q_PROPERTY(int close_icon_px READ get_close_icon_px WRITE set_close_icon_px)

    public:
        /**
         * @brief Constructs the tab bar.
         * @param parent The parent widget.
         */
        explicit TabBar(QWidget* parent = nullptr);

        /**
         * @brief Gets the normal (non-hover) close icon color.
         * @return The color used for the close icon.
         */
        [[nodiscard]] auto get_close_icon_color() const -> QColor;

        /**
         * @brief Sets the normal (non-hover) close icon color and refreshes icons.
         * @param color The new color to use.
         */
        auto set_close_icon_color(const QColor& color) -> void;

        /**
         * @brief Gets the hover/pressed close icon color.
         * @return The color used when hovering/pressing the close icon.
         */
        [[nodiscard]] auto get_close_icon_color_hover() const -> QColor;

        /**
         * @brief Sets the hover/pressed close icon color and refreshes icons.
         * @param color The new hover color to use.
         */
        auto set_close_icon_color_hover(const QColor& color) -> void;

        /**
         * @brief Gets the close icon size in pixels (square).
         * @return The icon size in pixels.
         */
        [[nodiscard]] auto get_close_icon_px() const -> int;

        /**
         * @brief Sets the close icon size in pixels (square) and refreshes icons.
         * @param px The icon size in pixels. Values < 1 are clamped to 1.
         */
        auto set_close_icon_px(int px) -> void;

    protected:
        /**
         * @brief Ensures the close button is set up when a tab is inserted.
         * @param index The index of the inserted tab.
         */
        void tabInserted(int index) override;

        /**
         * @brief Re-applies icons when the bar becomes visible.
         * @param event The show event.
         */
        void showEvent(QShowEvent* event) override;

        /**
         * @brief Handles hover/press events on dynamically created close buttons.
         * @param watched The watched object (expected: a close button).
         * @param event The event to handle.
         * @return true to consume; false to propagate (always false here).
         */
        bool eventFilter(QObject* watched, QEvent* event) override;

    private slots:
        /**
         * @brief Handles clicks on dynamically created close buttons.
         */
        auto handle_close_button_clicked() -> void;

    private:
        /**
         * @brief Updates the close button for a specific tab index.
         * @param index The tab index to update.
         */
        auto update_close_button_for(int index) -> void;

        /**
         * @brief Updates all existing close buttons.
         */
        auto update_all_close_buttons() -> void;

        /**
         * @brief Builds a colored close QIcon for the requested color.
         * @param color The color to tint the SVG with.
         * @return The resulting QIcon.
         */
        [[nodiscard]] auto make_icon(const QColor& color) const -> QIcon;

    private:
        QColor m_close_icon_color = QColor("#666666");
        QColor m_close_icon_color_hover = QColor("#42a5f5");
        int m_close_icon_px = 18;
};
