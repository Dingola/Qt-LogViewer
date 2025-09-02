#pragma once

#include <QDialog>
#include <QMouseEvent>
#include <QVBoxLayout>

#include "Qt-LogViewer/Views/DialogHeaderWidget.h"

/**
 * @class Dialog
 * @brief Base class for custom dialogs with a custom header and drag-move support.
 *
 * Inherit from this class for all dialogs that require a custom header (with close button)
 * and frameless window style. The header is styled via QSS and emits a signal when closed.
 * The dialog can be moved by dragging the header.
 */
class Dialog: public QDialog
{
        Q_OBJECT
        Q_PROPERTY(int border_radius READ get_border_radius WRITE set_border_radius)

    public:
        /**
         * @brief Constructs a Dialog with a custom header.
         * @param title The title to display in the header.
         * @param parent The parent widget.
         */
        explicit Dialog(const QString& title, QWidget* parent = nullptr);

		/**
		 * @brief Returns the current border radius.
		 * @return The border radius in pixels.
		 */
        [[nodiscard]] auto get_border_radius() const -> int;

		/**
		 * @brief Sets the border radius for rounded corners.
		 * @param radius The border radius in pixels.
		 */
        auto set_border_radius(int radius) -> void;

    protected:
        /**
         * @brief Handles mouse press events for drag-move.
         * @param event The mouse event.
         */
        void mousePressEvent(QMouseEvent* event) override;

        /**
         * @brief Handles mouse move events for drag-move.
         * @param event The mouse event.
         */
        void mouseMoveEvent(QMouseEvent* event) override;

        /**
         * @brief Handles mouse release events for drag-move.
         * @param event The mouse event.
         */
        void mouseReleaseEvent(QMouseEvent* event) override;

		/**
         * @brief Handles resize events to apply rounded corners.
         * @param event The resize event.
         */
        void resizeEvent(QResizeEvent* event) override;

        /**
         * @brief Returns the header widget.
         * @return Pointer to the DialogHeaderWidget.
         */
        auto header_widget() -> DialogHeaderWidget*;

    private:
		int m_border_radius = 16;			  ///< Border radius for rounded corners.
        DialogHeaderWidget* m_header_widget;  ///< Custom header widget.
        QPoint m_drag_position;               ///< Position for drag-move.
        bool m_dragging = false;              ///< Dragging state.
};
