#include "Qt-LogViewer/Views/Shared/TreeView.h"

#include <QPainter>
#include <QStyle>
#include <QStyleOption>

#include "Qt-LogViewer/Services/UiUtils.h"

/**
 * @file TreeView.cpp
 * @brief Implementation of TreeView. Customizes branch connector painting.
 *
 * Adjustments compared to default QTreeView:
 *  - No vertical connector segment below a CLOSED (collapsed) branch icon.
 *  - Expanded branches keep a vertical segment below the icon unless they are the last sibling.
 *  - Ancestor lines remain continuous for ancestors that have a following sibling.
 */

/**
 * @brief Constructs an empty TreeView with default properties.
 * @param parent Optional parent widget.
 *
 * Uses defaults defined in member initializers; no model is set here.
 */
TreeView::TreeView(QWidget* parent): QTreeView(parent) {}

/* ===== Setters ===== */
/**
 * @brief Updates the expanded (open) branch icon SVG path.
 * @param path Resource or filesystem path. Repaints only on change.
 */
auto TreeView::set_tree_view_branch_icon_open(const QString& path) -> void
{
    if (m_icon_open != path)
    {
        m_icon_open = path;
        request_repaint();
    }
}

/**
 * @brief Updates the collapsed (closed) branch icon SVG path.
 * @param path Resource or filesystem path. Repaints only on change.
 */
auto TreeView::set_tree_view_branch_icon_closed(const QString& path) -> void
{
    if (m_icon_closed != path)
    {
        m_icon_closed = path;
        request_repaint();
    }
}

/**
 * @brief Sets the tint color used for both connector lines and branch SVG indicators.
 * @param color Desired color. Repaints only on change.
 */
auto TreeView::set_tree_view_branch_icon_color(const QColor& color) -> void
{
    if (m_icon_color != color)
    {
        m_icon_color = color;
        request_repaint();
    }
}

/**
 * @brief Sets the indicator icon square size in pixels.
 * @param px Size; <=0 defers to indentation().
 *
 * Triggers repaint only if changed.
 */
auto TreeView::set_tree_view_branch_icon_px(int px) -> void
{
    if (m_icon_px != px)
    {
        m_icon_px = px;
        request_repaint();
    }
}

/**
 * @brief Sets the pen width for connector lines (clamped to minimum 1).
 * @param px Requested width; values <1 become 1.
 *
 * Repaints only if the stored (possibly clamped) value changes.
 */
auto TreeView::set_tree_view_branch_pen_width(int px) -> void
{
    int v = px;
    if (v < 1)
    {
        v = 1;
    }
    if (m_pen_width != v)
    {
        m_pen_width = v;
        request_repaint();
    }
}

/**
 * @brief Sets the horizontal connector length.
 * @param px Length in pixels; <=0 uses indentation()/2.
 *
 * Repaints only if changed.
 */
auto TreeView::set_tree_view_branch_hlength_px(int px) -> void
{
    if (m_h_len_px != px)
    {
        m_h_len_px = px;
        request_repaint();
    }
}

/**
 * @brief Enables or disables drawing a vertical segment above top-level items.
 * @param enabled True to draw; false to start at the icon center.
 */
auto TreeView::set_tree_view_branch_top_vline(bool enabled) -> void
{
    if (m_top_vline != enabled)
    {
        m_top_vline = enabled;
        request_repaint();
    }
}

/* ===== Getters ===== */
/**
 * @brief Returns the expanded (open) branch icon path.
 * @return SVG path string.
 */
auto TreeView::get_tree_view_branch_icon_open() const -> QString
{
    QString v = m_icon_open;
    return v;
}

/**
 * @brief Returns the collapsed (closed) branch icon path.
 * @return SVG path string.
 */
auto TreeView::get_tree_view_branch_icon_closed() const -> QString
{
    QString v = m_icon_closed;
    return v;
}

/**
 * @brief Returns the tint color applied to icons and lines.
 * @return QColor value.
 */
auto TreeView::get_tree_view_branch_icon_color() const -> QColor
{
    QColor v = m_icon_color;
    return v;
}

/**
 * @brief Returns the configured icon size.
 * @return Pixel size; <=0 means dynamic indentation-based sizing.
 */
auto TreeView::get_tree_view_branch_icon_px() const -> int
{
    int v = m_icon_px;
    return v;
}

/**
 * @brief Returns the connector pen width.
 * @return Width in pixels (>=1).
 */
auto TreeView::get_tree_view_branch_pen_width() const -> int
{
    int v = m_pen_width;
    return v;
}

/**
 * @brief Returns the horizontal connector length.
 * @return Pixels; <=0 indicates indentation()/2 usage.
 */
auto TreeView::get_tree_view_branch_hlength_px() const -> int
{
    int v = m_h_len_px;
    return v;
}

/**
 * @brief Returns whether top-level vertical segments are drawn above indicators.
 * @return True if enabled; false otherwise.
 */
auto TreeView::get_tree_view_branch_top_vline() const -> bool
{
    bool v = m_top_vline;
    return v;
}

/* ===== Painting ===== */
/**
 * @brief Draws branch connectors and indicators.
 *
 * Logic summary:
 *  - Collapsed nodes omit vertical line below icon.
 *  - Expanded nodes (non-last siblings) draw vertical continuation.
 *  - Leaf nodes draw vertical line to center (last) or full height (non-last).
 *  - Horizontal connector drawn from icon toward item up to configured length.
 *  - Ancestor vertical lines drawn only if ancestor has following siblings.
 *
 * @param painter Active QPainter (antialiasing enabled).
 * @param rect Branch rectangle provided by QTreeView.
 * @param index Model index for the item whose branch is being painted.
 */
void TreeView::drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const
{
    QTreeView::drawBranches(painter, rect, index);

    const int indent_px = indentation();
    int icon_px = m_icon_px > 0 ? m_icon_px : indent_px;
    if (icon_px <= 0)
    {
        icon_px = 12;
    }

    int h_len = m_h_len_px > 0 ? m_h_len_px : indent_px / 2;
    if (h_len < 4)
    {
        h_len = 4;
    }

    QVector<QModelIndex> ancestors;
    QModelIndex it = index.parent();
    while (it.isValid())
    {
        ancestors.prepend(it);
        it = it.parent();
    }
    int depth = ancestors.size();

    QModelIndex parent_index = index.parent();
    int sibling_count =
        parent_index.isValid() ? model()->rowCount(parent_index) : model()->rowCount(QModelIndex());
    bool is_last = index.row() >= (sibling_count - 1);
    bool has_children = model()->hasChildren(index);
    bool is_open = has_children && isExpanded(index);

    QPen pen(m_icon_color, m_pen_width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(pen);

    int y_top = rect.top();
    int y_mid = rect.center().y();
    int y_bot = rect.bottom();

    // Ancestor vertical lines
    for (int level = 0; level < depth; ++level)
    {
        QModelIndex anc = ancestors.at(level);
        QModelIndex anc_parent = anc.parent();
        int anc_siblings =
            anc_parent.isValid() ? model()->rowCount(anc_parent) : model()->rowCount(QModelIndex());
        bool anc_has_next = anc.row() < (anc_siblings - 1);
        if (anc_has_next)
        {
            int x = rect.left() + level * indent_px + (indent_px / 2);
            painter->drawLine(QPoint(x, y_top), QPoint(x, y_bot));
        }
    }

    int slot_x_center = rect.left() + depth * indent_px + (indent_px / 2);
    int vertical_start = y_top;
    if (!m_top_vline && !parent_index.isValid())
    {
        vertical_start = y_mid;
    }

    if (has_children)
    {
        int icon_w = qMin(icon_px, rect.width());
        int icon_h = qMin(icon_px, rect.height());
        int icon_x = slot_x_center - (icon_w / 2);
        int icon_y = y_mid - (icon_h / 2);
        QRect indicator_rect(icon_x, icon_y, icon_w, icon_h);

        int gap_v = (m_pen_width / 2) + 1;

        int above_end = indicator_rect.top() - gap_v;
        if (above_end > vertical_start)
        {
            painter->drawLine(QPoint(slot_x_center, vertical_start),
                              QPoint(slot_x_center, above_end));
        }

        // Below segment only if expanded AND not last sibling
        if (is_open && !is_last)
        {
            int below_start = indicator_rect.bottom() + gap_v;
            if (below_start < y_bot)
            {
                painter->drawLine(QPoint(slot_x_center, below_start), QPoint(slot_x_center, y_bot));
            }
        }

        // Horizontal connector to the right of icon
        int gap_h = (m_pen_width / 2) + 1;
        int horiz_start = indicator_rect.right() + gap_h;
        int horiz_end = qMin(rect.right(), slot_x_center + h_len);
        if (horiz_end > horiz_start)
        {
            painter->drawLine(QPoint(horiz_start, y_mid), QPoint(horiz_end, y_mid));
        }

        QString path = is_open ? m_icon_open : m_icon_closed;
        if (!path.isEmpty())
        {
            QPixmap pm = UiUtils::colored_svg_icon(path, m_icon_color, indicator_rect.size());
            painter->drawPixmap(indicator_rect.topLeft(), pm);
        }
    }
    else
    {
        // Leaf node connector
        if (is_last)
        {
            painter->drawLine(QPoint(slot_x_center, vertical_start), QPoint(slot_x_center, y_mid));
        }
        else
        {
            painter->drawLine(QPoint(slot_x_center, vertical_start), QPoint(slot_x_center, y_bot));
        }

        int horiz_end = qMin(rect.right(), slot_x_center + h_len);
        if (horiz_end > slot_x_center)
        {
            painter->drawLine(QPoint(slot_x_center, y_mid), QPoint(horiz_end, y_mid));
        }
    }

    painter->restore();
}

/* ===== Helpers ===== */
/**
 * @brief Requests a viewport repaint after property updates.
 *
 * Safe check prevents dereferencing a null viewport (e.g., during destruction).
 */
auto TreeView::request_repaint() -> void
{
    if (viewport() != nullptr)
    {
        viewport()->update();
    }
}
