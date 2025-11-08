#include "Qt-LogViewer/Views/Shared/DockWidget.h"

#include <QCloseEvent>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QShowEvent>
#include <QStyle>
#include <QToolButton>

#include "Qt-LogViewer/Services/UiUtils.h"

/**
 * @file DockWidget.cpp
 * @brief Implementation of DockWidget with a custom title bar and QSS-driven colored close icon.
 */

/**
 * @brief Creates a custom title bar widget for a QDockWidget.
 * @param dock_widget The dock widget to create the title bar for.
 * @param object_name The object name for QSS styling.
 * @return Pointer to the created QWidget.
 */
auto DockWidget::create_dock_title_bar(QDockWidget* dock_widget) -> QWidget*
{
    QWidget* result = nullptr;

    auto* title_bar = new QWidget(dock_widget);
    title_bar->setAttribute(Qt::WA_StyledBackground, true);
    title_bar->setObjectName(QStringLiteral("dock_title_bar"));

    auto* layout = new QHBoxLayout(title_bar);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    auto* label = new QLabel(dock_widget->windowTitle(), title_bar);
    label->setObjectName(QStringLiteral("dock_title_label"));
    layout->addWidget(label);
    layout->addStretch(1);

    auto* close_button = new QToolButton(title_bar);
    close_button->setObjectName(QStringLiteral("dock_title_bar_close_button"));
    close_button->setAutoRaise(true);

    auto* self = qobject_cast<DockWidget*>(dock_widget);

    if (self != nullptr)
    {
        self->m_close_button = close_button;
        close_button->setIcon(self->make_close_icon(self->m_close_icon_color));
        close_button->setIconSize(QSize(self->m_close_icon_px, self->m_close_icon_px));
        close_button->installEventFilter(self);
    }
    else
    {
        // Fallback standard icon if cast fails.
        close_button->setIcon(
            title_bar->style()->standardIcon(QStyle::SP_TitleBarCloseButton, nullptr, title_bar));
    }

    QObject::connect(close_button, &QToolButton::clicked, dock_widget, &QDockWidget::close);
    layout->addWidget(close_button);

    result = title_bar;
    return result;
}

/**
 * @brief Gets the normal (non-hover) close icon color.
 * @return The color used for the close icon.
 */
auto DockWidget::get_close_icon_color() const -> QColor
{
    QColor result = m_close_icon_color;
    return result;
}

/**
 * @brief Sets the normal (non-hover) close icon color and refreshes the icon.
 * @param color The new color to use.
 */
auto DockWidget::set_close_icon_color(const QColor& color) -> void
{
    if (m_close_icon_color != color)
    {
        m_close_icon_color = color;
        update_close_button();
    }
}

/**
 * @brief Gets the hover/pressed close icon color.
 * @return The color used when hovering/pressing the close icon.
 */
auto DockWidget::get_close_icon_color_hover() const -> QColor
{
    return m_close_icon_color_hover;
}

/**
 * @brief Sets the hover/pressed close icon color and refreshes the icon.
 * @param color The new hover color to use.
 */
auto DockWidget::set_close_icon_color_hover(const QColor& color) -> void
{
    if (m_close_icon_color_hover != color)
    {
        m_close_icon_color_hover = color;
        update_close_button();
    }
}

/**
 * @brief Gets the close icon size in pixels (square).
 * @return The icon size in pixels.
 */
auto DockWidget::get_close_icon_px() const -> int
{
    return m_close_icon_px;
}

/**
 * @brief Sets the close icon size in pixels (square) and refreshes the icon.
 * @param px The icon size in pixels. Values < 1 are clamped to 1.
 */
auto DockWidget::set_close_icon_px(int px) -> void
{
    int clamped = px;

    if (clamped < 1)
    {
        clamped = 1;
    }
    if (m_close_icon_px != clamped)
    {
        m_close_icon_px = clamped;
        update_close_button();
    }
}

/**
 * @brief Re-applies the icon when the widget is shown.
 * @param event The show event.
 */
void DockWidget::showEvent(QShowEvent* event)
{
    QDockWidget::showEvent(event);
    update_close_button();
}

/**
 * @brief Handles hover/press events on the close button.
 * @param watched The watched object (expected: the close button).
 * @param event The event to handle.
 * @return true if consumed; false otherwise (always false).
 */
bool DockWidget::eventFilter(QObject* watched, QEvent* event)
{
    bool handled = false;
    auto* btn = qobject_cast<QToolButton*>(watched);

    if (btn != nullptr && btn == m_close_button)
    {
        bool enter = (event->type() == QEvent::Enter);
        bool leave = (event->type() == QEvent::Leave);
        bool press = (event->type() == QEvent::MouseButtonPress);
        bool release = (event->type() == QEvent::MouseButtonRelease);

        if (enter || press)
        {
            btn->setIcon(make_close_icon(m_close_icon_color_hover));
        }
        else
        {
            if (leave || release)
            {
                btn->setIcon(make_close_icon(m_close_icon_color));
            }
        }
    }
    return handled;
}

/**
 * @brief Updates the existing close button (if any) with current properties.
 */
auto DockWidget::update_close_button() -> void
{
    if (m_close_button != nullptr)
    {
        m_close_button->setIcon(make_close_icon(m_close_icon_color));
        m_close_button->setIconSize(QSize(m_close_icon_px, m_close_icon_px));
    }
}

/**
 * @brief Builds a colored close QIcon for the requested color.
 * @param color The color to tint the SVG with.
 * @return The resulting QIcon.
 */
auto DockWidget::make_close_icon(const QColor& color) const -> QIcon
{
    auto result = QIcon(UiUtils::colored_svg_icon(QStringLiteral(":/Resources/Icons/close.svg"),
                                                  color, QSize(m_close_icon_px, m_close_icon_px)));
    return result;
}

/**
 * @brief Handles the close event.
 * @param event The close event.
 */
void DockWidget::closeEvent(QCloseEvent* event)
{
    emit closed();
    QDockWidget::closeEvent(event);
}
