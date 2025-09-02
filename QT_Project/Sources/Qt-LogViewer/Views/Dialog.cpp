/**
 * @file Dialog.cpp
 * @brief Implementation of the Dialog base class for custom Qt dialogs with header and drag-move
 * support.
 *
 * This file contains the implementation of the Dialog class, which provides a frameless window,
 * a custom header with close button, and drag-move functionality for all derived dialogs.
 * See Dialog.h for interface details.
 */
#include "Qt-LogViewer/Views/Dialog.h"

#include <QPainterPath>
#include <QRegion>
#include <QResizeEvent>

/**
 * @brief Constructs a Dialog with a custom header.
 * @param title The title to display in the header.
 * @param parent The parent widget.
 */
Dialog::Dialog(const QString& title, QWidget* parent)
    : QDialog(parent), m_header_widget(new DialogHeaderWidget(title, this))
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground, false);
    setObjectName("Dialog");

    connect(m_header_widget, &DialogHeaderWidget::close_requested, this, &QDialog::reject);
}

/**
 * @brief Returns the current border radius.
 * @return The border radius in pixels.
 */
auto Dialog::get_border_radius() const -> int
{
    return m_border_radius;
}

/**
 * @brief Sets the border radius for rounded corners.
 * @param radius The border radius in pixels.
 */
auto Dialog::set_border_radius(int radius) -> void
{
    m_border_radius = radius; update();
}

/**
 * @brief Handles mouse press events for drag-move.
 * @param event The mouse event.
 */
void Dialog::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton &&
        m_header_widget->rect().contains(m_header_widget->mapFromParent(event->pos())))
    {
        m_dragging = true;
        m_drag_position = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }

    QDialog::mousePressEvent(event);
}

/**
 * @brief Handles mouse move events for drag-move.
 * @param event The mouse event.
 */
void Dialog::mouseMoveEvent(QMouseEvent* event)
{
    if (m_dragging && (event->buttons() & Qt::LeftButton))
    {
        move(event->globalPosition().toPoint() - m_drag_position);
        event->accept();
    }

    QDialog::mouseMoveEvent(event);
}

/**
 * @brief Handles mouse release events for drag-move.
 * @param event The mouse event.
 */
void Dialog::mouseReleaseEvent(QMouseEvent* event)
{
    m_dragging = false;
    QDialog::mouseReleaseEvent(event);
}

/**
 * @brief Handles resize events to apply rounded corners.
 * @param event The resize event.
 */
void Dialog::resizeEvent(QResizeEvent* event)
{
    QDialog::resizeEvent(event);

    QPainterPath path;
    path.addRoundedRect(rect(), get_border_radius(), get_border_radius());
    QRegion mask = QRegion(path.toFillPolygon().toPolygon());
    setMask(mask);
}

/**
 * @brief Returns the header widget.
 * @return Pointer to the DialogHeaderWidget.
 */
auto Dialog::header_widget() -> DialogHeaderWidget*
{
    return m_header_widget;
}
