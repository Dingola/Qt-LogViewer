#pragma once

#include <QColor>
#include <QTabWidget>

class QEvent;
class QMenu;
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
 *  - qproperty-enable_default_context_menu
 *
 * Example:
 *   TabWidget {
 *       qproperty-close_icon_color: #ffffff;
 *       qproperty-close_icon_color_hover: #42a5f5;
 *       qproperty-close_icon_px: 18;
 *       qproperty-enable_default_context_menu: true;
 *   }
 *
 * Context menu:
 *  - For convenience, this widget forwards the tab context menu extensibility/signals from TabBar.
 *  - Additionally, TabWidget performs the default close operations when actions are selected.
 *
 * Translation:
 *  - changeEvent() is implemented to support dynamic language switching; currently nothing to
 * retranslate locally, but the override is in place for future UI strings.
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
        /**
         * @brief Enables/disables the built-in default tab context menu. Forwarded to TabBar.
         */
        Q_PROPERTY(bool enable_default_context_menu READ get_enable_default_context_menu WRITE
                       set_enable_default_context_menu)

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

        /**
         * @brief Gets whether the built-in default context menu is enabled. Forwarded to TabBar.
         * @return true if enabled; false otherwise.
         */
        [[nodiscard]] auto get_enable_default_context_menu() const -> bool;

        /**
         * @brief Enables or disables the built-in default context menu. Forwarded to TabBar.
         * @param enabled New enabled state.
         */
        auto set_enable_default_context_menu(bool enabled) -> void;

    signals:
        /**
         * @brief Forwarded: emitted before the tab context menu is shown.
         * @param tab_index Index of the tab under the cursor, or -1 if none.
         * @param menu Pointer to the QMenu being prepared.
         */
        void about_to_show_context_menu(int tab_index, QMenu* menu);

        /**
         * @brief Forwarded: requested to close the current tab.
         * @param tab_index Index of the tab to close.
         */
        void close_tab_requested(int tab_index);

        /**
         * @brief Forwarded: requested to close all other tabs than the current one.
         * @param tab_index Index of the tab to keep.
         */
        void close_other_tabs_requested(int tab_index);

        /**
         * @brief Forwarded: requested to close tabs to the left of the current one.
         * @param tab_index Rightmost tab to keep.
         */
        void close_tabs_to_left_requested(int tab_index);

        /**
         * @brief Forwarded: requested to close tabs to the right of the current one.
         * @param tab_index Leftmost tab to keep.
         */
        void close_tabs_to_right_requested(int tab_index);

    protected:
        /**
         * @brief Re-translates persistent UI texts on language changes.
         * @param event The change event.
         */
        void changeEvent(QEvent* event) override;

    private slots:
        /**
         * @brief Default handler: remove the requested tab.
         * @param tab_index Index of the tab to close.
         */
        auto handle_close_tab_requested(int tab_index) -> void;

        /**
         * @brief Default handler: close all tabs except the given one.
         * @param tab_index Index of the tab to keep.
         */
        auto handle_close_other_tabs_requested(int tab_index) -> void;

        /**
         * @brief Default handler: close tabs to the left of the given one.
         * @param tab_index Rightmost tab to keep.
         */
        auto handle_close_tabs_to_left_requested(int tab_index) -> void;

        /**
         * @brief Default handler: close tabs to the right of the given one.
         * @param tab_index Leftmost tab to keep.
         */
        auto handle_close_tabs_to_right_requested(int tab_index) -> void;
};
