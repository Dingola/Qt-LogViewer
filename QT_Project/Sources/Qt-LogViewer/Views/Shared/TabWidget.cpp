/**
 * @file TabWidget.cpp
 * @brief Implementation of TabWidget using TabBar with QSS-driven colored close icons.
 */

#include "Qt-LogViewer/Views/Shared/TabWidget.h"

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
