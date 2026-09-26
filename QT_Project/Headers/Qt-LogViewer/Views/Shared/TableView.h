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
        Q_PROPERTY(QColor fatal_color MEMBER m_fatal_color)
        Q_PROPERTY(QColor error_color MEMBER m_error_color)
        Q_PROPERTY(QColor warning_color MEMBER m_warning_color)
        Q_PROPERTY(QColor info_color MEMBER m_info_color)
        Q_PROPERTY(QColor debug_color MEMBER m_debug_color)
        Q_PROPERTY(QColor trace_color MEMBER m_trace_color)
        Q_PROPERTY(QColor hover_row_color READ get_hover_row_color WRITE set_hover_row_color)
        Q_PROPERTY(
            QColor search_match_color READ get_search_match_color WRITE set_search_match_color)

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
        [[nodiscard]] auto get_hover_row_color() const -> QColor;

        /**
         * @brief Sets the hover row color.
         * @param color The new hover color.
         */
        auto set_hover_row_color(const QColor& color) -> void;

        /**
         * @brief Returns the accent used for search-match borders and translucent fills.
         * @return Current theme's search-match color.
         */
        [[nodiscard]] auto get_search_match_color() const -> QColor;

        /**
         * @brief Sets the search-match accent and repaints the viewport when it changes.
         * @param color Search-match accent supplied by the stylesheet.
         */
        auto set_search_match_color(const QColor& color) -> void;

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
        QColor m_fatal_color;    ///< Stylesheet foreground for fatal records.
        QColor m_error_color;    ///< Stylesheet foreground for error records.
        QColor m_warning_color;  ///< Stylesheet foreground for warning records.
        QColor m_info_color;     ///< Stylesheet foreground for info records.
        QColor m_debug_color;    ///< Stylesheet foreground for debug records.
        QColor m_trace_color;    ///< Stylesheet foreground for trace records.
        QColor m_hover_row_color = QColor("#25384a");
        QColor m_search_match_color = QColor("#7aa0ba");
};
