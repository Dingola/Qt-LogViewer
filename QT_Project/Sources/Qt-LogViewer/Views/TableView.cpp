#include "Qt-LogViewer/Views/TableView.h"

#include <QMouseEvent>

/**
 * @brief Constructs a TableView object.
 * @param parent The parent widget, or nullptr.
 */
TableView::TableView(QWidget* parent): QTableView(parent) {}

/**
 * @brief Returns the current hover row color.
 * @return The color used for hover row highlighting.
 */
auto TableView::hover_row_color() const -> QColor
{
    return m_hover_row_color;
}

/**
 * @brief Sets the hover row color.
 * @param color The new hover color.
 */
auto TableView::set_hover_row_color(const QColor& color) -> void
{
    m_hover_row_color = color;
}

/**
 * @brief Handles mouse move events to emit the hover index signal.
 * @param event The mouse event.
 */
void TableView::mouseMoveEvent(QMouseEvent* event)
{
    emit hover_index_changed(indexAt(event->pos()));
    QTableView::mouseMoveEvent(event);
}

/**
 * @brief Handles leave events to reset the hover index.
 * @param event The leave event.
 */
void TableView::leaveEvent(QEvent* event)
{
    emit hover_index_changed(QModelIndex());
    QTableView::leaveEvent(event);
}
