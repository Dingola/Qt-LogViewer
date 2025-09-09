#include "Qt-LogViewer/Views/Shared/DialogHeaderWidget.h"

#include <QStyle>

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
    : QWidget(parent),
      m_title_label(new QLabel(title, this)),
      m_close_button(new QPushButton(this)),
      m_layout(new QHBoxLayout(this))
{
    m_layout->addWidget(m_title_label);
    m_layout->addStretch();
    m_layout->addWidget(m_close_button);
    setLayout(m_layout);

    setObjectName("DialogHeaderWidget");
    m_close_button->setObjectName("DialogHeaderCloseButton");
    m_close_button->setFixedSize(24, 24);
    m_close_button->setFlat(true);
    m_close_button->setIcon(style()->standardIcon(QStyle::SP_TitleBarCloseButton, nullptr, this));

    connect(m_close_button, &QPushButton::clicked, this, &DialogHeaderWidget::close_requested);
}

/**
 * @brief Sets the title of the header.
 * @param title The new title to display.
 */
auto DialogHeaderWidget::set_title(const QString& title) -> void
{
    m_title_label->setText(title);
}
