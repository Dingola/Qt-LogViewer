#include "Qt-LogViewer/Views/Shared/DialogHeaderWidget.h"

#include <QStyle>

#include "QtWidgetsCommonLib/Utils/UiUtils.h"

/**
 * @file DialogHeaderWidget.cpp
 * @brief Implementation of DialogHeaderWidget for custom dialog headers.
 */

/**
 * @brief Constructs a DialogHeaderWidget.
 * @param title The title to display in the header.
 * @param parent The parent widget.
 */
DialogHeaderWidget::DialogHeaderWidget(const QString& title, QWidget* parent)
    : QFrame(parent),
      m_title_label(new QLabel(title, this)),
      m_close_button(new QPushButton(this)),
      m_layout(new QHBoxLayout(this))
{
    setAttribute(Qt::WA_StyledBackground, true);

    m_layout->addWidget(m_title_label);
    m_layout->addStretch();
    m_layout->addWidget(m_close_button);
    m_layout->setContentsMargins(0, 0, 0, 0);
    setLayout(m_layout);

    setObjectName("DialogHeaderWidget");
    m_close_button->setObjectName("DialogHeaderCloseButton");
    m_close_button->setFixedSize(28, 28);
    m_close_button->setFlat(true);
    m_close_button->setIconSize(QSize(18, 18));
    set_close_icon_color(m_close_icon_color);

    connect(m_close_button, &QPushButton::clicked, this, &DialogHeaderWidget::close_requested);
}

/**
 * @brief Returns the current title.
 * @return The current title text.
 */
auto DialogHeaderWidget::get_title() const -> QString
{
    return m_title_label->text();
}

/**
 * @brief Sets the title of the header.
 * @param title The new title to display.
 */
auto DialogHeaderWidget::set_title(const QString& title) -> void
{
    m_title_label->setText(title);
}

/**
 * @brief Returns the color used to render the title-bar close SVG.
 * @return Current close-icon color.
 */
auto DialogHeaderWidget::close_icon_color() const -> QColor
{
    return m_close_icon_color;
}

/**
 * @brief Recolors the close SVG using the same resource and size as the main window.
 * @param color Icon color supplied by the stylesheet.
 */
auto DialogHeaderWidget::set_close_icon_color(const QColor& color) -> void
{
    m_close_icon_color = color;
    if (!m_close_icon.isEmpty())
    {
        m_close_button->setIcon(
            QtWidgetsCommonLib::UiUtils::colored_svg_icon(m_close_icon, color, QSize(18, 18)));
    }
}

/**
 * @brief Returns the close icon resource path.
 * @return Current stylesheet property value.
 */
auto DialogHeaderWidget::close_icon() const -> QString
{
    return m_close_icon;
}

/**
 * @brief Sets the close icon resource path.
 * @param value New stylesheet property value.
 */
auto DialogHeaderWidget::set_close_icon(const QString& value) -> void
{
    if (m_close_icon != value)
    {
        m_close_icon = value;
        set_close_icon_color(m_close_icon_color);
    }
}