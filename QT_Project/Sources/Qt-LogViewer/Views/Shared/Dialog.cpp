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
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QResizeEvent>

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
    setObjectName("Dialog");

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
}

/**
 * @brief Handles mouse press events for drag-move.
 * @param event The mouse event.
 */
void Dialog::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_header_widget->isVisible() &&
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
    update();
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
        auto* content_wrapper = new QWidget(this);
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
