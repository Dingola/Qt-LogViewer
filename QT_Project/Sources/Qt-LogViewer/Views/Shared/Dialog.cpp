/**
 * @file Dialog.cpp
 * @brief Implementation of the Dialog base class for custom Qt dialogs with header and drag-move
 * support.
 *
 * This file contains the implementation of the Dialog class, which provides a frameless window,
 * a custom header with close button, and drag-move functionality for all derived dialogs.
 * See Dialog.h for interface details.
 */
#include "Qt-LogViewer/Views/Shared/Dialog.h"

#include <QDebug>
#include <QHoverEvent>
#include <QLayout>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QResizeEvent>
#include <QTimer>

/**
 * @brief Constructs a Dialog with a custom header.
 * @param title The title to display in the header.
 * @param parent The parent widget.
 * @param mode The layout management mode.
 */
Dialog::Dialog(const QString& title, QWidget* parent, LayoutMode mode)
    : QDialog(parent), m_layout_mode(mode), m_header_widget(new DialogHeaderWidget(title, this))
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_Hover, true);
    setObjectName("Dialog");
    setMouseTracking(true);

    connect(m_header_widget, &DialogHeaderWidget::close_requested, this, &QDialog::reject);

    if (m_layout_mode == LayoutMode::Managed)
    {
        initialize_managed_layout();
        m_header_initialized = true;
    }
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
    if (radius >= 0 && m_border_radius != radius)
    {
        m_border_radius = radius;
        update();
    }
}

/**
 * @brief Returns whether the header is visible.
 * @return true if the header is visible, false otherwise.
 */
auto Dialog::is_header_visible() const -> bool
{
    return m_header_widget->isVisible();
}

/**
 * @brief Sets the visibility of the header widget.
 * @param visible true to show the header, false to hide it.
 */
auto Dialog::set_header_visible(bool visible) -> void
{
    m_header_widget->setVisible(visible);
}

/**
 * @brief Returns the background color.
 * @return The current background color.
 */
auto Dialog::get_background_color() const -> QColor
{
    return m_background_color;
}

/**
 * @brief Sets the background color.
 * @param color The background color.
 */
auto Dialog::set_background_color(const QColor& color) -> void
{
    m_background_color = color;
    update();
}

/**
 * @brief Returns the border color.
 * @return The current border color.
 */
auto Dialog::get_border_color() const -> QColor
{
    return m_border_color;
}

/**
 * @brief Sets the border color.
 * @param color The border color.
 */
auto Dialog::set_border_color(const QColor& color) -> void
{
    m_border_color = color;
    update();
}

/**
 * @brief Returns the border width.
 * @return The border width in pixels.
 */
auto Dialog::get_border_width() const -> int
{
    return m_border_width;
}

/**
 * @brief Sets the border width.
 * @param width The border width in pixels.
 */
auto Dialog::set_border_width(int width) -> void
{
    m_border_width = width;
    update();
}

/**
 * @brief Returns whether the dialog is resizable.
 * @return true if resizable, false otherwise.
 */
auto Dialog::is_resizable() const -> bool
{
    return m_resizable;
}

/**
 * @brief Sets whether the dialog is resizable.
 * @param resizable true to enable resizing, false to disable.
 */
auto Dialog::set_resizable(bool resizable) -> void
{
    m_resizable = resizable;
    if (!m_resizable)
    {
        setCursor(Qt::ArrowCursor);
    }
}

/**
 * @brief Returns the title of the dialog header.
 * @return The current title text.
 */
auto Dialog::get_title() const -> QString
{
    return m_header_widget->get_title();
}

/**
 * @brief Sets the title of the dialog header.
 * @param title The new title to display.
 */
auto Dialog::set_title(const QString& title) -> void
{
    m_header_widget->set_title(title);
}

/**
 * @brief Returns the content layout (only for Managed mode).
 * @return Pointer to the content layout, or nullptr in External mode.
 */
auto Dialog::content_layout() -> QVBoxLayout*
{
    QVBoxLayout* result = nullptr;

    if (m_layout_mode == LayoutMode::Managed)
    {
        result = m_content_layout;
    }
    else
    {
        qWarning() << "Dialog::content_layout() called in External mode. Use existing layout.";
    }

    return result;
}

/**
 * @brief Paints the dialog with rounded corners and border.
 * @param event The paint event.
 */
void Dialog::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (m_border_radius > 0)
    {
        const qreal half_border = m_border_width / 2.0;

        // Background path with full radius
        QPainterPath background_path;
        background_path.addRoundedRect(rect(), m_border_radius, m_border_radius);
        painter.fillPath(background_path, m_background_color);

        // Border path with adjusted radius (subtract half border width from radius)
        QPainterPath border_path;
        QRectF border_rect =
            QRectF(rect()).adjusted(half_border, half_border, -half_border, -half_border);
        const qreal adjusted_radius = qMax(0.0, m_border_radius - half_border);
        border_path.addRoundedRect(border_rect, adjusted_radius, adjusted_radius);

        painter.setPen(QPen(m_border_color, m_border_width));
        painter.setBrush(Qt::NoBrush);
        painter.drawPath(border_path);
    }
    else
    {
        painter.fillRect(rect(), m_background_color);
        const qreal half_border = m_border_width / 2.0;
        painter.setPen(QPen(m_border_color, m_border_width));
        painter.drawRect(
            QRectF(rect()).adjusted(half_border, half_border, -half_border, -half_border));
    }
}

/**
 * @brief Handles the first show event to finalize layout setup.
 * @param event The show event.
 */
void Dialog::showEvent(QShowEvent* event)
{
    if (!m_header_initialized && m_layout_mode == LayoutMode::External)
    {
        initialize_header_for_existing_layout();
    }

    QDialog::showEvent(event);

    auto enforce_acceptable_size = [this]() {
        if (layout() != nullptr)
        {
            layout()->activate();
        }

        const QSize acceptable_size = QLayout::closestAcceptableSize(this, size());
        if (acceptable_size != size())
        {
            resize(acceptable_size);
        }
    };

    enforce_acceptable_size();

    QTimer::singleShot(0, this, [this, enforce_acceptable_size]() { enforce_acceptable_size(); });
}

/**
 * @brief Handles mouse press events for drag-move and resize.
 * @param event The mouse event.
 */
void Dialog::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_resize_edges = get_resize_edge(event->pos());

        if (m_resizable && m_resize_edges != Qt::Edges())
        {
            m_resizing = true;
            m_resize_start_pos = event->globalPosition().toPoint();
            m_resize_start_geometry = geometry();

            m_resize_minimum_size = minimumSizeHint().expandedTo(minimumSize());
            m_resize_maximum_size = maximumSize();

            event->accept();
        }
        else if (is_in_draggable_header_area(event->pos()))
        {
            m_dragging = true;
            m_drag_position = event->globalPosition().toPoint() - frameGeometry().topLeft();
            event->accept();
        }
        else
        {
            QDialog::mousePressEvent(event);
        }
    }
    else
    {
        QDialog::mousePressEvent(event);
    }
}

/**
 * @brief Handles mouse move events for drag-move and resize.
 * @param event The mouse event.
 */
void Dialog::mouseMoveEvent(QMouseEvent* event)
{
    if (m_resizing && (event->buttons() & Qt::LeftButton))
    {
        const QPoint delta = event->globalPosition().toPoint() - m_resize_start_pos;
        QRect new_geometry = m_resize_start_geometry;

        if (m_resize_edges & Qt::LeftEdge)
        {
            new_geometry.setLeft(m_resize_start_geometry.left() + delta.x());
        }
        if (m_resize_edges & Qt::RightEdge)
        {
            new_geometry.setRight(m_resize_start_geometry.right() + delta.x());
        }
        if (m_resize_edges & Qt::TopEdge)
        {
            new_geometry.setTop(m_resize_start_geometry.top() + delta.y());
        }
        if (m_resize_edges & Qt::BottomEdge)
        {
            new_geometry.setBottom(m_resize_start_geometry.bottom() + delta.y());
        }

        const QSize accepted_size = QLayout::closestAcceptableSize(this, new_geometry.size());

        if (m_resize_edges & Qt::LeftEdge)
        {
            new_geometry.setLeft(new_geometry.right() - accepted_size.width() + 1);
        }
        else if (m_resize_edges & Qt::RightEdge)
        {
            new_geometry.setRight(new_geometry.left() + accepted_size.width() - 1);
        }

        if (m_resize_edges & Qt::TopEdge)
        {
            new_geometry.setTop(new_geometry.bottom() - accepted_size.height() + 1);
        }
        else if (m_resize_edges & Qt::BottomEdge)
        {
            new_geometry.setBottom(new_geometry.top() + accepted_size.height() - 1);
        }

        if (new_geometry != geometry())
        {
            setGeometry(new_geometry);
        }

        event->accept();
    }
    else if (m_dragging && (event->buttons() & Qt::LeftButton))
    {
        move(event->globalPosition().toPoint() - m_drag_position);
        event->accept();
    }
    else
    {
        QDialog::mouseMoveEvent(event);
    }
}

/**
 * @brief Handles mouse release events for drag-move and resize.
 * @param event The mouse event.
 */
void Dialog::mouseReleaseEvent(QMouseEvent* event)
{
    m_dragging = false;
    m_resizing = false;
    m_resize_edges = Qt::Edges();
    QDialog::mouseReleaseEvent(event);
}

/**
 * @brief Handles resize events to apply rounded corners.
 * @param event The resize event.
 */
void Dialog::resizeEvent(QResizeEvent* event)
{
    QDialog::resizeEvent(event);
    update();
}

/**
 * @brief Handles hover events to update cursor for resize.
 * @param event The hover event.
 */
bool Dialog::event(QEvent* event)
{
    if (event->type() == QEvent::HoverMove)
    {
        auto* hover_event = static_cast<QHoverEvent*>(event);
        if (m_resizable && !m_dragging && !m_resizing)
        {
            Qt::Edges edges = get_resize_edge(hover_event->position().toPoint());
            update_cursor_shape(edges);
        }
    }
    else if (event->type() == QEvent::Leave)
    {
        if (m_resizable && !m_dragging && !m_resizing)
        {
            setCursor(Qt::ArrowCursor);
        }
    }

    return QDialog::event(event);
}

/**
 * @brief Returns the header widget (const).
 * @return Pointer to the DialogHeaderWidget.
 */
auto Dialog::header_widget() const -> const DialogHeaderWidget*
{
    return m_header_widget;
}

/**
 * @brief Initializes the dialog structure with managed layout (header + content area).
 */
auto Dialog::initialize_managed_layout() -> void
{
    m_main_layout = new QVBoxLayout(this);
    m_main_layout->setContentsMargins(0, 0, 0, 0);
    m_main_layout->setSpacing(0);

    m_main_layout->addWidget(m_header_widget);

    m_content_layout = new QVBoxLayout();
    m_content_layout->setContentsMargins(16, 16, 16, 16);
    m_content_layout->setSpacing(8);

    m_main_layout->addLayout(m_content_layout);
}

/**
 * @brief Initializes the header for dialogs with external layouts.
 */
auto Dialog::initialize_header_for_existing_layout() -> void
{
    auto* existing_layout = qobject_cast<QVBoxLayout*>(layout());
    if (existing_layout != nullptr)
    {
        auto* content_wrapper = new QFrame(this);
        content_wrapper->setAttribute(Qt::WA_StyledBackground, true);
        content_wrapper->setObjectName("DialogContent");
        content_wrapper->setLayout(existing_layout);

        auto* new_main_layout = new QVBoxLayout(this);
        new_main_layout->setContentsMargins(0, 0, 0, 0);
        new_main_layout->setSpacing(0);

        new_main_layout->addWidget(m_header_widget);
        new_main_layout->addWidget(content_wrapper);

        setLayout(new_main_layout);

        m_header_initialized = true;
    }
    else
    {
        qWarning() << "Dialog: No QVBoxLayout found. Header cannot be added.";
    }
}

/**
 * @brief Determines the resize edge from cursor position.
 * @param pos The cursor position relative to the dialog.
 * @return Qt::Edges flags indicating which edges are being resized.
 */
auto Dialog::get_resize_edge(const QPoint& pos) const -> Qt::Edges
{
    if (!m_resizable)
    {
        return Qt::Edges();
    }

    Qt::Edges edges;
    const QRect rect_area = rect();

    if (pos.x() <= RESIZE_MARGIN)
    {
        edges |= Qt::LeftEdge;
    }
    if (pos.x() >= rect_area.width() - RESIZE_MARGIN)
    {
        edges |= Qt::RightEdge;
    }
    if (pos.y() <= RESIZE_MARGIN)
    {
        edges |= Qt::TopEdge;
    }
    if (pos.y() >= rect_area.height() - RESIZE_MARGIN)
    {
        edges |= Qt::BottomEdge;
    }

    return edges;
}

/**
 * @brief Updates the cursor shape based on resize edge.
 * @param edges The resize edges.
 */
auto Dialog::update_cursor_shape(Qt::Edges edges) -> void
{
    if (m_resizable)
    {
        if (edges == (Qt::LeftEdge | Qt::TopEdge) || edges == (Qt::RightEdge | Qt::BottomEdge))
        {
            setCursor(Qt::SizeFDiagCursor);
        }
        else if (edges == (Qt::RightEdge | Qt::TopEdge) || edges == (Qt::LeftEdge | Qt::BottomEdge))
        {
            setCursor(Qt::SizeBDiagCursor);
        }
        else if (edges & (Qt::LeftEdge | Qt::RightEdge))
        {
            setCursor(Qt::SizeHorCursor);
        }
        else if (edges & (Qt::TopEdge | Qt::BottomEdge))
        {
            setCursor(Qt::SizeVerCursor);
        }
        else
        {
            setCursor(Qt::ArrowCursor);
        }
    }
    else
    {
        setCursor(Qt::ArrowCursor);
    }
}

/**
 * @brief Checks if a point is within the header area for drag detection.
 * @param pos The position to check (dialog coordinates).
 * @return true if the point is in a draggable header area, false otherwise.
 */
auto Dialog::is_in_draggable_header_area(const QPoint& pos) const -> bool
{
    return m_header_widget->isVisible() &&
           m_header_widget->rect().contains(m_header_widget->mapFromParent(pos));
}
