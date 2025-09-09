#pragma once

#include <QColor>
#include <QTableView>

/**
 * @class TableView
 * @brief Extended QTableView with support for hover row highlighting.
 *
 * This class adds a Q_PROPERTY for the hover row color and emits a signal
 * with the current hovered index on mouse movement.
 */
class TableView: public QTableView
{
        Q_OBJECT
        Q_PROPERTY(QColor hover_row_color READ get_hover_row_color WRITE set_hover_row_color)

    public:
        /**
         * @brief Constructs a TableView object.
         * @param parent The parent widget, or nullptr.
         */
        explicit TableView(QWidget* parent = nullptr);

        /**
         * @brief Returns the current hover row color.
         * @return The color used for hover row highlighting.
         */
        auto get_hover_row_color() const -> QColor;

        /**
         * @brief Sets the hover row color.
         * @param color The new hover color.
         */
        auto set_hover_row_color(const QColor& color) -> void;

    signals:
        /**
         * @brief Emitted when the mouse moves over a new row.
         * @param index The index of the currently hovered cell (or an invalid index).
         */
        void hover_index_changed(const QModelIndex& index);

    protected:
        /**
         * @brief Handles mouse move events to emit the hover index signal.
         * @param event The mouse event.
         */
        void mouseMoveEvent(QMouseEvent* event) override;

        /**
         * @brief Handles leave events to reset the hover index.
         * @param event The leave event.
         */
        void leaveEvent(QEvent* event) override;

    private:
        QColor m_hover_row_color = QColor("#25384a");
};
