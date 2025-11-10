/**
 * @file TabWidget.cpp
 * @brief Implementation of TabWidget using TabBar with QSS-driven colored close icons.
 */

#include "Qt-LogViewer/Views/Shared/TabWidget.h"

#include <QEvent>
#include <QMenu>
#include <QTabBar>

#include "Qt-LogViewer/Views/Shared/TabBar.h"

/**
 * @brief Constructs the tab widget and installs a TabBar.
 * @param parent The parent widget.
 */
TabWidget::TabWidget(QWidget* parent): QTabWidget(parent)
{
    setTabsClosable(true);
    auto* bar = new TabBar(this);
    setTabBar(bar);

    // Remove tabs when the close button on a tab is clicked (QTabWidget's own signal).
    connect(this, &QTabWidget::tabCloseRequested, this, &TabWidget::handle_close_tab_requested);

    // Default close behavior for context menu actions emitted by TabBar.
    connect(bar, &TabBar::close_tab_requested, this, &TabWidget::handle_close_tab_requested);
    connect(bar, &TabBar::close_other_tabs_requested, this,
            &TabWidget::handle_close_other_tabs_requested);
    connect(bar, &TabBar::close_tabs_to_left_requested, this,
            &TabWidget::handle_close_tabs_to_left_requested);
    connect(bar, &TabBar::close_tabs_to_right_requested, this,
            &TabWidget::handle_close_tabs_to_right_requested);

    // Forward TabBar's extensibility and actions through TabWidget signals for external listeners.
    connect(bar, &TabBar::about_to_show_context_menu, this, &TabWidget::about_to_show_context_menu);
    connect(bar, &TabBar::close_tab_requested, this, &TabWidget::close_tab_requested);
    connect(bar, &TabBar::close_other_tabs_requested, this, &TabWidget::close_other_tabs_requested);
    connect(bar, &TabBar::close_tabs_to_left_requested, this,
            &TabWidget::close_tabs_to_left_requested);
    connect(bar, &TabBar::close_tabs_to_right_requested, this,
            &TabWidget::close_tabs_to_right_requested);
}

/**
 * @brief Gets the normal close icon color.
 * @return The color used for the close icon.
 */
[[nodiscard]] auto TabWidget::get_close_icon_color() const -> QColor
{
    auto result = QColor("#666666");
    auto* bar = qobject_cast<TabBar*>(tabBar());

    if (bar != nullptr)
    {
        result = bar->get_close_icon_color();
    }

    return result;
}

/**
 * @brief Sets the normal close icon color.
 * @param color The new color to use.
 */
auto TabWidget::set_close_icon_color(const QColor& color) -> void
{
    auto* bar = qobject_cast<TabBar*>(tabBar());

    if (bar != nullptr)
    {
        bar->set_close_icon_color(color);
    }
}

/**
 * @brief Gets the hover/pressed close icon color.
 * @return The hover/pressed color used for the close icon.
 */
[[nodiscard]] auto TabWidget::get_close_icon_color_hover() const -> QColor
{
    auto result = QColor("#42a5f5");
    auto* bar = qobject_cast<TabBar*>(tabBar());

    if (bar != nullptr)
    {
        result = bar->get_close_icon_color_hover();
    }

    return result;
}

/**
 * @brief Sets the hover/pressed close icon color.
 * @param color The new hover/pressed color to use.
 */
auto TabWidget::set_close_icon_color_hover(const QColor& color) -> void
{
    auto* bar = qobject_cast<TabBar*>(tabBar());

    if (bar != nullptr)
    {
        bar->set_close_icon_color_hover(color);
    }
}

/**
 * @brief Gets the close icon size in pixels (square).
 * @return The icon size in pixels.
 */
[[nodiscard]] auto TabWidget::get_close_icon_px() const -> int
{
    int result = 18;
    auto* bar = qobject_cast<TabBar*>(tabBar());

    if (bar != nullptr)
    {
        result = bar->get_close_icon_px();
    }

    return result;
}

/**
 * @brief Sets the close icon size in pixels (square).
 * @param px The icon size in pixels.
 */
auto TabWidget::set_close_icon_px(int px) -> void
{
    auto* bar = qobject_cast<TabBar*>(tabBar());

    if (bar != nullptr)
    {
        bar->set_close_icon_px(px);
    }
}

/**
 * @brief Gets whether the built-in default context menu is enabled. Forwarded to TabBar.
 * @return true if enabled; false otherwise.
 */
[[nodiscard]] auto TabWidget::get_enable_default_context_menu() const -> bool
{
    bool result = true;
    auto* bar = qobject_cast<TabBar*>(tabBar());

    if (bar != nullptr)
    {
        result = bar->get_enable_default_context_menu();
    }

    return result;
}

/**
 * @brief Enables or disables the built-in default context menu. Forwarded to TabBar.
 * @param enabled New enabled state.
 */
auto TabWidget::set_enable_default_context_menu(bool enabled) -> void
{
    auto* bar = qobject_cast<TabBar*>(tabBar());

    if (bar != nullptr)
    {
        bar->set_enable_default_context_menu(enabled);
    }
}

/**
 * @brief Re-translates persistent UI texts on language changes.
 * @param event The change event.
 */
void TabWidget::changeEvent(QEvent* event)
{
    if (event != nullptr && event->type() == QEvent::LanguageChange)
    {
        // No persistent texts here currently; TabBar updates its own tooltips/menu texts.
    }

    QTabWidget::changeEvent(event);
}

/**
 * @brief Default handler: remove the requested tab.
 * @param tab_index Index of the tab to close.
 */
auto TabWidget::handle_close_tab_requested(int tab_index) -> void
{
    int idx = tab_index;

    if (idx >= 0 && idx < count())
    {
        removeTab(idx);
    }
}

/**
 * @brief Default handler: close all tabs except the given one.
 * @param tab_index Index of the tab to keep.
 */
auto TabWidget::handle_close_other_tabs_requested(int tab_index) -> void
{
    int keep = tab_index;

    if (keep >= 0 && keep < count())
    {
        for (int i = count() - 1; i >= 0; --i)
        {
            if (i != keep)
            {
                removeTab(i);
            }
        }
    }
}

/**
 * @brief Default handler: close tabs to the left of the given one.
 * @param tab_index Rightmost tab to keep.
 */
auto TabWidget::handle_close_tabs_to_left_requested(int tab_index) -> void
{
    int idx = tab_index;

    if (idx > 0)
    {
        for (int i = idx - 1; i >= 0; --i)
        {
            removeTab(i);
        }
    }
}

/**
 * @brief Default handler: close tabs to the right of the given one.
 * @param tab_index Leftmost tab to keep.
 */
auto TabWidget::handle_close_tabs_to_right_requested(int tab_index) -> void
{
    int idx = tab_index;

    if (idx >= 0 && idx < (count() - 1))
    {
        for (int i = count() - 1; i > idx; --i)
        {
            removeTab(i);
        }
    }
}
