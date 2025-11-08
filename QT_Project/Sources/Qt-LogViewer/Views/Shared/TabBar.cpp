/**
 * @file TabBar.cpp
 * @brief Implementation of TabBar with QSS-driven colored close icons.
 */

#include "Qt-LogViewer/Views/Shared/TabBar.h"

#include <QAbstractButton>
#include <QEvent>
#include <QPushButton>
#include <QShowEvent>
#include <QToolButton>

#include "Qt-LogViewer/Services/UiUtils.h"

/**
 * @brief Constructs the tab bar.
 * @param parent The parent widget.
 */
TabBar::TabBar(QWidget* parent): QTabBar(parent)
{
    setDrawBase(false);
    setDocumentMode(true);
}

/**
 * @brief Gets the normal (non-hover) close icon color.
 * @return The color used for the close icon.
 */
[[nodiscard]] auto TabBar::get_close_icon_color() const -> QColor
{
    QColor result = m_close_icon_color;
    return result;
}

/**
 * @brief Sets the normal (non-hover) close icon color and refreshes icons.
 * @param color The new color to use.
 */
auto TabBar::set_close_icon_color(const QColor& color) -> void
{
    if (m_close_icon_color != color)
    {
        m_close_icon_color = color;
        update_all_close_buttons();
    }
}

/**
 * @brief Gets the hover/pressed close icon color.
 * @return The color used when hovering/pressing the close icon.
 */
[[nodiscard]] auto TabBar::get_close_icon_color_hover() const -> QColor
{
    return m_close_icon_color_hover;
}

/**
 * @brief Sets the hover/pressed close icon color and refreshes icons.
 * @param color The new hover color to use.
 */
auto TabBar::set_close_icon_color_hover(const QColor& color) -> void
{
    if (m_close_icon_color_hover != color)
    {
        m_close_icon_color_hover = color;
        update_all_close_buttons();
    }
}

/**
 * @brief Gets the close icon size in pixels (square).
 * @return The icon size in pixels.
 */
[[nodiscard]] auto TabBar::get_close_icon_px() const -> int
{
    return m_close_icon_px;
}

/**
 * @brief Sets the close icon size in pixels (square) and refreshes icons.
 * @param px The icon size in pixels. Values < 1 are clamped to 1.
 */
auto TabBar::set_close_icon_px(int px) -> void
{
    int clamped = px;

    if (clamped < 1)
    {
        clamped = 1;
    }

    if (m_close_icon_px != clamped)
    {
        m_close_icon_px = clamped;
        update_all_close_buttons();
    }
}

/**
 * @brief Ensures the close button is set up when a tab is inserted.
 * @param index The index of the inserted tab.
 */
void TabBar::tabInserted(int index)
{
    QTabBar::tabInserted(index);
    update_close_button_for(index);
}

/**
 * @brief Re-applies icons when the bar becomes visible.
 * @param event The show event.
 */
void TabBar::showEvent(QShowEvent* event)
{
    QTabBar::showEvent(event);
    update_all_close_buttons();
}

/**
 * @brief Handles hover/press events on dynamically created close buttons.
 * @param watched The watched object (expected: a close button).
 * @param event The event to handle.
 * @return true to consume; false to propagate (always false here).
 */
bool TabBar::eventFilter(QObject* watched, QEvent* event)
{
    bool handled = false;
    auto* btn = qobject_cast<QAbstractButton*>(watched);

    if (btn != nullptr)
    {
        bool enter = (event->type() == QEvent::Enter);
        bool leave = (event->type() == QEvent::Leave);
        bool press = (event->type() == QEvent::MouseButtonPress);
        bool release = (event->type() == QEvent::MouseButtonRelease);

        if (enter || press)
        {
            btn->setIcon(make_icon(m_close_icon_color_hover));
        }
        else
        {
            if (leave || release)
            {
                btn->setIcon(make_icon(m_close_icon_color));
            }
        }
    }

    return handled;
}

/**
 * @brief Handles clicks on dynamically created close buttons.
 */
auto TabBar::handle_close_button_clicked() -> void
{
    auto* sender_btn = qobject_cast<QAbstractButton*>(sender());
    int index_to_close = -1;

    if (sender_btn != nullptr)
    {
        for (int i = 0; i < count() && index_to_close < 0; ++i)
        {
            if (tabButton(i, QTabBar::RightSide) == sender_btn)
            {
                index_to_close = i;
            }
        }
    }

    if (index_to_close >= 0)
    {
        emit tabCloseRequested(index_to_close);
    }
}

/**
 * @brief Updates the close button for a specific tab index.
 * @param index The tab index to update.
 */
auto TabBar::update_close_button_for(int index) -> void
{
    bool tabs_closeable = tabsClosable();
    QToolButton* tool_btn = nullptr;

    if (tabs_closeable)
    {
        tool_btn = new QToolButton(this);
        tool_btn->setAutoRaise(true);
        tool_btn->setCursor(Qt::ArrowCursor);
        tool_btn->setObjectName(QStringLiteral("tabbar_close_button"));

        const QSize icon_sz(m_close_icon_px, m_close_icon_px);
        tool_btn->setIcon(make_icon(m_close_icon_color));
        tool_btn->setIconSize(icon_sz);

        connect(tool_btn, &QToolButton::clicked, this, &TabBar::handle_close_button_clicked);

        setTabButton(index, QTabBar::RightSide, tool_btn);
        tool_btn->removeEventFilter(this);
        tool_btn->installEventFilter(this);
        tool_btn->setIcon(make_icon(m_close_icon_color));
        tool_btn->setAutoRaise(true);
        tool_btn->setIconSize(icon_sz);
        tool_btn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        tool_btn->setContentsMargins(0, 0, 0, 0);
    }
}

/**
 * @brief Updates all existing close buttons.
 */
auto TabBar::update_all_close_buttons() -> void
{
    for (int i = 0; i < count(); ++i)
    {
        update_close_button_for(i);
    }
}

/**
 * @brief Builds a colored close QIcon for the requested color.
 * @param color The color to tint the SVG with.
 * @return The resulting QIcon.
 */
[[nodiscard]] auto TabBar::make_icon(const QColor& color) const -> QIcon
{
    QSize sz(m_close_icon_px, m_close_icon_px);
    auto result = QIcon(UiUtils::colored_svg_icon(":/Resources/Icons/close.svg", color, sz));

    return result;
}
