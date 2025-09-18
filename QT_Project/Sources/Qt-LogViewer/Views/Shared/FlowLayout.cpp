/**
 * @file FlowLayout.cpp
 * @brief Implementation of FlowLayout, a layout that arranges child widgets horizontally and wraps
 * them to new lines as needed.
 *
 * Supports configurable horizontal alignment for each row.
 */

#include "Qt-LogViewer/Views/Shared/FlowLayout.h"

#include <QLayoutItem>
#include <QStyle>
#include <QWidget>
#include <algorithm>

/**
 * @brief Constructs a FlowLayout.
 * @param parent The parent widget.
 * @param margin The margin around the layout.
 * @param h_spacing The horizontal spacing between items.
 * @param v_spacing The vertical spacing between rows.
 * @param alignment The row alignment (default: Left).
 */
FlowLayout::FlowLayout(QWidget* parent, int margin, int h_spacing, int v_spacing,
                       RowAlignment alignment)
    : QLayout(parent), m_h_space(h_spacing), m_v_space(v_spacing), m_row_alignment(alignment)
{
    setContentsMargins(margin, margin, margin, margin);
}

/**
 * @brief Destroys the FlowLayout.
 */
FlowLayout::~FlowLayout()
{
    while (!m_item_list.isEmpty())
    {
        delete m_item_list.takeFirst();
    }
}

/**
 * @brief Adds an item to the layout.
 * @param item The layout item to add.
 */
void FlowLayout::addItem(QLayoutItem* item)
{
    m_item_list.append(item);
}

/**
 * @brief Returns the number of items in the layout.
 * @return The number of items.
 */
auto FlowLayout::count() const -> int
{
    return m_item_list.size();
}

/**
 * @brief Returns the item at the given index.
 * @param index The index of the item.
 * @return The item at the given index, or nullptr if out of bounds.
 */
auto FlowLayout::itemAt(int index) const -> QLayoutItem*
{
    QLayoutItem* result = nullptr;

    if (index >= 0 && index < m_item_list.size())
    {
        result = m_item_list.at(index);
    }

    return result;
}

/**
 * @brief Removes and returns the item at the given index.
 * @param index The index of the item.
 * @return The removed item, or nullptr if out of bounds.
 */
auto FlowLayout::takeAt(int index) -> QLayoutItem*
{
    QLayoutItem* result = nullptr;

    if (index >= 0 && index < m_item_list.size())
    {
        result = m_item_list.takeAt(index);
    }

    return result;
}

/**
 * @brief Returns the preferred size for the layout.
 * If m_expand_to_show_all_rows is true, height is calculated to fit all rows.
 */
auto FlowLayout::sizeHint() const -> QSize
{
    QSize size;

    for (const QLayoutItem* item: m_item_list)
    {
        size = size.expandedTo(item->sizeHint());
    }

    int left = 0, top = 0, right = 0, bottom = 0;
    getContentsMargins(&left, &top, &right, &bottom);

    if (m_expand_to_show_all_rows)
    {
        // Calculate the full height needed for all rows
        QRect dummy_rect(0, 0, size.width(), 10000);  // Large height to force all rows
        int total_height = do_layout(dummy_rect, true);
        size.setHeight(total_height);
    }

    size += QSize(left + right, top + bottom);
    return size;
}

/**
 * @brief Returns the minimum size for the layout.
 * @return The minimum size.
 */
auto FlowLayout::minimumSize() const -> QSize
{
    QSize size;

    for (const QLayoutItem* item: m_item_list)
    {
        size = size.expandedTo(item->minimumSize());
    }

    int left = 0, top = 0, right = 0, bottom = 0;
    getContentsMargins(&left, &top, &right, &bottom);
    size += QSize(left + right, top + bottom);
    return size;
}

/**
 * @brief Sets the geometry of the layout.
 * @param rect The rectangle to set.
 */
void FlowLayout::setGeometry(const QRect& rect)
{
    QLayout::setGeometry(rect);
    do_layout(rect, false);
}

/**
 * @brief Returns the horizontal spacing between items.
 * @return The horizontal spacing (never negative).
 */
auto FlowLayout::horizontal_spacing() const -> int
{
    int result = m_h_space;

    if (result < 0)
    {
        result = smart_spacing(QStyle::PM_LayoutHorizontalSpacing);

        if (result < 0)
        {
            result = 0;
        }
    }

    return result;
}

/**
 * @brief Returns the vertical spacing between rows.
 * @return The vertical spacing (never negative).
 */
auto FlowLayout::vertical_spacing() const -> int
{
    int result = m_v_space;

    if (result < 0)
    {
        result = smart_spacing(QStyle::PM_LayoutVerticalSpacing);

        if (result < 0)
        {
            result = 0;
        }
    }

    return result;
}

/**
 * @brief Sets the row alignment (left or center).
 * @param alignment The desired row alignment.
 */
auto FlowLayout::set_row_alignment(RowAlignment alignment) -> void
{
    m_row_alignment = alignment;
    invalidate();
}

/**
 * @brief Returns the current row alignment.
 * @return The current row alignment.
 */
auto FlowLayout::get_row_alignment() const -> RowAlignment
{
    return m_row_alignment;
}

/**
 * @brief Sets whether the layout should expand vertically to show all rows.
 * @param expand If true, layout expands to show all rows; otherwise, fits to parent height.
 */
auto FlowLayout::set_expand_to_show_all_rows(bool expand) -> void
{
    m_expand_to_show_all_rows = expand;
    invalidate();
}

/**
 * @brief Returns whether the layout expands vertically to show all rows.
 */
auto FlowLayout::get_expand_to_show_all_rows() const -> bool
{
    return m_expand_to_show_all_rows;
}

/**
 * @brief Performs the layout of items.
 * @param rect The rectangle to layout within.
 * @param test_only If true, only calculates layout size, does not set geometry.
 * @return The total height used.
 *
 * RowAlignment::Left: rows are left-aligned.
 * RowAlignment::Center: rows are always centered.
 * RowAlignment::CenterLeft: all rows are centered, but rows with fewer items are left-aligned with
 * the widest row above.
 */
auto FlowLayout::do_layout(const QRect& rect, bool test_only) const -> int
{
    int left = 0, top = 0, right = 0, bottom = 0;
    getContentsMargins(&left, &top, &right, &bottom);

    int x = rect.x() + left;
    int y = rect.y() + top;
    int line_height = 0;
    int start_x = x;
    int max_width = rect.width() - left - right;

    QList<QLayoutItem*> line_items;
    int line_width = 0;

    // Track the left edge of the first centered row
    int first_row_left_x = -1;
    bool is_first_row = true;

    for (int i = 0; i < m_item_list.size(); ++i)
    {
        QLayoutItem* item = m_item_list.at(i);
        int space_x = horizontal_spacing();
        int space_y = vertical_spacing();
        int item_width = item->sizeHint().width();
        int item_height = item->sizeHint().height();
        int next_x = x + item_width + space_x;

        if ((next_x - space_x > rect.right()) && (line_height > 0))
        {
            int offset_x = 0;

            if (m_row_alignment == RowAlignment::CenterLeft)
            {
                if (is_first_row)
                {
                    offset_x = (max_width - line_width + space_x) / 2;
                    first_row_left_x = start_x + offset_x;
                    is_first_row = false;
                }
                else
                {
                    offset_x = first_row_left_x - start_x;
                }
            }
            else if (m_row_alignment == RowAlignment::Center)
            {
                offset_x = (max_width - line_width + space_x) / 2;
            }
            // Left: offset_x remains 0

            for (int j = 0; j < line_items.size(); ++j)
            {
                if (!test_only)
                {
                    QRect geom = line_items.at(j)->geometry();
                    geom.moveLeft(geom.left() + offset_x);
                    line_items.at(j)->setGeometry(geom);
                }
            }

            line_items.clear();
            line_width = 0;

            x = start_x;
            y += line_height + space_y;
            next_x = x + item_width + space_x;
            line_height = 0;
        }

        if (!test_only)
        {
            item->setGeometry(QRect(QPoint(x, y), item->sizeHint()));
        }

        line_items.append(item);
        line_width += item_width + space_x;
        x = next_x;
        line_height = std::max(line_height, item_height);
    }

    // Handle last line
    if (!line_items.isEmpty())
    {
        int offset_x = 0;

        if (m_row_alignment == RowAlignment::CenterLeft)
        {
            if (is_first_row)
            {
                offset_x = (max_width - line_width + horizontal_spacing()) / 2;
                first_row_left_x = start_x + offset_x;
            }
            else
            {
                offset_x = first_row_left_x - start_x;
            }
        }
        else if (m_row_alignment == RowAlignment::Center)
        {
            offset_x = (max_width - line_width + horizontal_spacing()) / 2;
        }
        // Left: offset_x remains 0

        for (int j = 0; j < line_items.size(); ++j)
        {
            if (!test_only)
            {
                QRect geom = line_items.at(j)->geometry();
                geom.moveLeft(geom.left() + offset_x);
                line_items.at(j)->setGeometry(geom);
            }
        }
    }

    int result = y + line_height - rect.y();
    return result;
}

/**
 * @brief Returns the smart spacing for the given pixel metric.
 * @param pm The pixel metric.
 * @return The spacing value.
 */
auto FlowLayout::smart_spacing(QStyle::PixelMetric pm) const -> int
{
    int result = -1;
    QObject* parent_obj = parent();

    if (parent_obj)
    {
        if (parent_obj->isWidgetType())
        {
            auto parent_widget = static_cast<QWidget*>(parent_obj);
            result = parent_widget->style()->pixelMetric(pm, nullptr, parent_widget);
        }
        else
        {
            result = static_cast<QLayout*>(parent_obj)->spacing();
        }
    }

    return result;
}
