#pragma once

#include <QLayout>
#include <QRect>
#include <QStyle>
#include <QWidgetItem>

/**
 * @class FlowLayout
 * @brief A layout that arranges child widgets horizontally and wraps them to new lines as needed.
 *
 * Supports configurable horizontal alignment for each row.
 */
class FlowLayout: public QLayout
{
    public:
        /**
         * @brief Alignment options for rows in the flow layout.
         */
        enum class RowAlignment
        {
            Left,       ///< Rows are left-aligned (default)
            Center,     ///< Rows are horizontally centered
            CenterLeft  ///< All rows are centered, but rows with fewer items are left-aligned with
                        ///< the widest row above
        };

        /**
         * @brief Constructs a FlowLayout.
         * @param parent The parent widget.
         * @param margin The margin around the layout.
         * @param h_spacing The horizontal spacing between items.
         * @param v_spacing The vertical spacing between rows.
         * @param alignment The row alignment (default: Left).
         */
        explicit FlowLayout(QWidget* parent = nullptr, int margin = -1, int h_spacing = -1,
                            int v_spacing = -1, RowAlignment alignment = RowAlignment::Left);

        /**
         * @brief Destroys the FlowLayout.
         */
        ~FlowLayout() override;

        /**
         * @brief Adds an item to the layout.
         * @param item The layout item to add.
         */
        void addItem(QLayoutItem* item) override;

        /**
         * @brief Returns the number of items in the layout.
         */
        [[nodiscard]] auto count() const -> int override;

        /**
         * @brief Returns the item at the given index.
         * @param index The index of the item.
         */
        [[nodiscard]] auto itemAt(int index) const -> QLayoutItem* override;

        /**
         * @brief Removes and returns the item at the given index.
         * @param index The index of the item.
         */
        auto takeAt(int index) -> QLayoutItem* override;

        /**
         * @brief Returns the preferred size for the layout.
         * If m_expand_to_show_all_rows is true, height is calculated to fit all rows.
         */
        [[nodiscard]] auto sizeHint() const -> QSize override;

        /**
         * @brief Returns the minimum size for the layout.
         */
        [[nodiscard]] auto minimumSize() const -> QSize override;

        /**
         * @brief Sets the geometry of the layout.
         * @param rect The rectangle to set.
         */
        void setGeometry(const QRect& rect) override;

        /**
         * @brief Returns the horizontal spacing between items.
         */
        [[nodiscard]] auto horizontal_spacing() const -> int;

        /**
         * @brief Returns the vertical spacing between rows.
         */
        [[nodiscard]] auto vertical_spacing() const -> int;

        /**
         * @brief Sets the row alignment (left or center).
         * @param alignment The desired row alignment.
         */
        auto set_row_alignment(RowAlignment alignment) -> void;

        /**
         * @brief Returns the current row alignment.
         */
        [[nodiscard]] auto get_row_alignment() const -> RowAlignment;

        /**
         * @brief Sets whether the layout should expand vertically to show all rows.
         * @param expand If true, layout expands to show all rows; otherwise, fits to parent height.
         */
        auto set_expand_to_show_all_rows(bool expand) -> void;

        /**
         * @brief Returns whether the layout expands vertically to show all rows.
         */
        [[nodiscard]] auto get_expand_to_show_all_rows() const -> bool;

    private:
        /**
         * @brief Performs the layout of items.
         * @param rect The rectangle to layout within.
         * @param test_only If true, only calculates layout size, does not set geometry.
         * @return The total height used.
         *
         * RowAlignment::Left: rows are left-aligned.
         * RowAlignment::Center: rows are always centered.
         * RowAlignment::CenterLeft: all rows are centered, but rows with fewer items are
         * left-aligned with the widest row above.
         */
        auto do_layout(const QRect& rect, bool test_only) const -> int;

        /**
         * @brief Returns the smart spacing for the given pixel metric.
         * @param pm The pixel metric.
         */
        [[nodiscard]] auto smart_spacing(QStyle::PixelMetric pm) const -> int;

    private:
        QList<QLayoutItem*> m_item_list;
        int m_h_space;
        int m_v_space;
        RowAlignment m_row_alignment;
        /**
         * @brief If true, layout expands vertically to show all rows; otherwise, fits to parent
         * height.
         */
        bool m_expand_to_show_all_rows = false;
};
