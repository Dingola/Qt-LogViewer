#pragma once

#include <QColor>
#include <QDialog>
#include <QMouseEvent>
#include <QShowEvent>
#include <QSize>
#include <QVBoxLayout>

#include "Qt-LogViewer/Views/Shared/DialogHeaderWidget.h"

/**
 * @class Dialog
 * @brief Base class for custom dialogs with a custom header and drag-move support.
 *
 * Inherit from this class for all dialogs that require a custom header (with close button)
 * and frameless window style. The header is styled via QSS and emits a signal when closed.
 * The dialog can be moved by dragging the header and resized from edges/corners.
 *
 * This class provides two usage modes:
 * 1. **Managed Layout Mode**: Dialog manages the layout and header automatically.
 * 2. **External Layout Mode**: Dialog automatically integrates with Qt Designer layouts.
 *
 * @section Usage Example 1: Managed Layout (Programmatic UI)
 * @code
 * class MyDialog : public Dialog
 * {
 *     Q_OBJECT
 * public:
 *     explicit MyDialog(QWidget* parent = nullptr)
 *         : Dialog(tr("My Dialog"), parent)
 *     {
 *         auto* widget = new QWidget(this);
 *         content_layout()->addWidget(widget);
 *     }
 * };
 * @endcode
 *
 * @section Usage Example 2: External Layout (Qt Designer UI)
 * @code
 * class MyDialog : public Dialog
 * {
 *     Q_OBJECT
 * public:
 *     explicit MyDialog(QWidget* parent = nullptr)
 *         : Dialog(tr("My Dialog"), parent, LayoutMode::External)
 *     {
 *         ui->setupUi(this);
 *     }
 * };
 * @endcode
 *
 * @note In External mode, the header is automatically inserted when the dialog is first shown.
 */
class Dialog: public QDialog
{
        Q_OBJECT
        Q_PROPERTY(int border_radius READ get_border_radius WRITE set_border_radius)
        Q_PROPERTY(bool header_visible READ is_header_visible WRITE set_header_visible)
        Q_PROPERTY(QColor background_color READ get_background_color WRITE set_background_color)
        Q_PROPERTY(QColor border_color READ get_border_color WRITE set_border_color)
        Q_PROPERTY(int border_width READ get_border_width WRITE set_border_width)
        Q_PROPERTY(bool resizable READ is_resizable WRITE set_resizable)

    public:
        /**
         * @enum LayoutMode
         * @brief Defines how the dialog manages its layout.
         */
        enum class LayoutMode
        {
            Managed,  ///< Dialog creates and manages the layout (default).
            External  ///< Dialog uses an existing layout (e.g., from Qt Designer).
        };

        /**
         * @brief Constructs a Dialog with a custom header.
         * @param title The title to display in the header.
         * @param parent The parent widget, or nullptr.
         * @param mode The layout management mode.
         */
        explicit Dialog(const QString& title, QWidget* parent = nullptr,
                        LayoutMode mode = LayoutMode::Managed);

        /**
         * @brief Virtual destructor for proper cleanup in derived classes.
         */
        ~Dialog() override = default;

        /**
         * @brief Returns the current border radius.
         * @return The border radius in pixels.
         */
        [[nodiscard]] auto get_border_radius() const -> int;

        /**
         * @brief Sets the border radius for rounded corners.
         * @param radius The border radius in pixels (must be >= 0).
         */
        auto set_border_radius(int radius) -> void;

        /**
         * @brief Returns whether the header is visible.
         * @return true if the header is visible, false otherwise.
         */
        [[nodiscard]] auto is_header_visible() const -> bool;

        /**
         * @brief Sets the visibility of the header widget.
         * @param visible true to show the header, false to hide it.
         */
        auto set_header_visible(bool visible) -> void;

        /**
         * @brief Returns the background color.
         * @return The current background color.
         */
        [[nodiscard]] auto get_background_color() const -> QColor;

        /**
         * @brief Sets the background color.
         * @param color The background color.
         */
        auto set_background_color(const QColor& color) -> void;

        /**
         * @brief Returns the border color.
         * @return The current border color.
         */
        [[nodiscard]] auto get_border_color() const -> QColor;

        /**
         * @brief Sets the border color.
         * @param color The border color.
         */
        auto set_border_color(const QColor& color) -> void;

        /**
         * @brief Returns the border width.
         * @return The border width in pixels.
         */
        [[nodiscard]] auto get_border_width() const -> int;

        /**
         * @brief Sets the border width.
         * @param width The border width in pixels.
         */
        auto set_border_width(int width) -> void;

        /**
         * @brief Returns whether the dialog is resizable.
         * @return true if resizable, false otherwise.
         */
        [[nodiscard]] auto is_resizable() const -> bool;

        /**
         * @brief Sets whether the dialog is resizable.
         * @param resizable true to enable resizing, false to disable.
         */
        auto set_resizable(bool resizable) -> void;

        /**
         * @brief Returns the title of the dialog header.
         * @return The current title text.
         */
        [[nodiscard]] auto get_title() const -> QString;

        /**
         * @brief Sets the title of the dialog header.
         * @param title The new title to display.
         */
        auto set_title(const QString& title) -> void;

        /**
         * @brief Returns the content layout (only for Managed mode).
         * @return Pointer to the content layout, or nullptr in External mode.
         */
        [[nodiscard]] auto content_layout() -> QVBoxLayout*;

    protected:
        /**
         * @brief Paints the dialog with rounded corners and border.
         * @param event The paint event.
         */
        void paintEvent(QPaintEvent* event) override;

        /**
         * @brief Handles the first show event to finalize layout setup.
         * @param event The show event.
         */
        void showEvent(QShowEvent* event) override;

        /**
         * @brief Handles mouse press events for drag-move and resize.
         * @param event The mouse event.
         */
        void mousePressEvent(QMouseEvent* event) override;

        /**
         * @brief Handles mouse move events for drag-move and resize.
         * @param event The mouse event.
         */
        void mouseMoveEvent(QMouseEvent* event) override;

        /**
         * @brief Handles mouse release events for drag-move and resize.
         * @param event The mouse event.
         */
        void mouseReleaseEvent(QMouseEvent* event) override;

        /**
         * @brief Handles resize events to apply rounded corners.
         * @param event The resize event.
         */
        void resizeEvent(QResizeEvent* event) override;

        /**
         * @brief Handles hover events to update cursor for resize.
         * @param event The hover event.
         */
        bool event(QEvent* event) override;

        /**
         * @brief Returns the header widget.
         * @return Pointer to the DialogHeaderWidget.
         */
        [[nodiscard]] auto header_widget() const -> const DialogHeaderWidget*;

    private:
        /**
         * @brief Initializes the dialog structure with managed layout (header + content area).
         */
        auto initialize_managed_layout() -> void;

        /**
         * @brief Initializes the header for dialogs with external layouts.
         */
        auto initialize_header_for_existing_layout() -> void;

        /**
         * @brief Determines the resize edge from cursor position.
         * @param pos The cursor position relative to the dialog.
         * @return Qt::Edges flags indicating which edges are being resized.
         */
        [[nodiscard]] auto get_resize_edge(const QPoint& pos) const -> Qt::Edges;

        /**
         * @brief Updates the cursor shape based on resize edge.
         * @param edges The resize edges.
         */
        auto update_cursor_shape(Qt::Edges edges) -> void;

        /**
         * @brief Checks if a point is within the header area for drag detection.
         * @param pos The position to check (dialog coordinates).
         * @return true if the point is in a draggable header area, false otherwise.
         */
        [[nodiscard]] auto is_in_draggable_header_area(const QPoint& pos) const -> bool;

    private:
        static constexpr int DEFAULT_BORDER_RADIUS = 0;  ///< Default border radius in pixels.
        static constexpr int DEFAULT_BORDER_WIDTH = 2;   ///< Default border width in pixels.
        static constexpr int RESIZE_MARGIN = 8;          ///< Margin for resize detection in pixels.

        LayoutMode m_layout_mode = LayoutMode::Managed;  ///< Layout management mode.
        int m_border_radius = DEFAULT_BORDER_RADIUS;     ///< Border radius for rounded corners.
        int m_border_width = DEFAULT_BORDER_WIDTH;       ///< Border width in pixels.
        QColor m_background_color{26, 32, 33};           ///< Background color.
        QColor m_border_color{48, 53, 58};               ///< Border color.
        DialogHeaderWidget* m_header_widget = nullptr;   ///< Custom header widget.
        QVBoxLayout* m_main_layout = nullptr;            ///< Main layout (Managed mode only).
        QVBoxLayout* m_content_layout = nullptr;         ///< Content layout (Managed mode only).
        QPoint m_drag_position;                          ///< Position for drag-move.
        bool m_dragging = false;                         ///< Dragging state.
        bool m_resizing = false;                         ///< Resizing state.
        bool m_resizable = true;                         ///< Whether dialog is resizable.
        Qt::Edges m_resize_edges = Qt::Edges();          ///< Current resize edges.
        QPoint m_resize_start_pos;                       ///< Start position for resize.
        QRect m_resize_start_geometry;                   ///< Start geometry for resize.
        bool m_header_initialized = false;               ///< Whether header has been initialized.
        QSize m_resize_minimum_size;                     ///< Cached minimum size during resize.
        QSize m_resize_maximum_size;                     ///< Cached maximum size during resize.
};
