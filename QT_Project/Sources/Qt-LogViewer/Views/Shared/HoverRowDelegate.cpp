#include "Qt-LogViewer/Views/Shared/HoverRowDelegate.h"

#include <QApplication>
#include <QPainter>

#include "Qt-LogViewer/Models/LogModel.h"
#include "Qt-LogViewer/Models/LogSortFilterProxyModel.h"
#include "Qt-LogViewer/Views/Shared/TableView.h"

/**
 * @brief Constructs a HoverRowDelegate object.
 * @param parent The parent QObject, or nullptr.
 */
HoverRowDelegate::HoverRowDelegate(QObject* parent): QStyledItemDelegate(parent) {}

/**
 * @brief Sets the currently hovered row.
 * @param row The row index to highlight, or -1 for none.
 */
auto HoverRowDelegate::set_hovered_row(int row) -> void
{
    if (m_hovered_row != row)
    {
        m_hovered_row = row;
    }
}

/**
 * @brief Returns the currently hovered row.
 * @return The hovered row index, or -1 if none.
 */
auto HoverRowDelegate::hovered_row() const -> int
{
    return m_hovered_row;
}

/**
 * @brief Lightweight paint implementation with precise highlights.
 *
 * - Paints hover background for the hovered row (if applicable).
 * - Requests highlight ranges via `HighlightRangesRole` and paints translucent rectangles
 *   using QFontMetrics for precise glyph positions.
 * - Draws default item content via base delegate.
 *
 * Highlights are rendered even for selected rows.
 *
 * @param painter The painter to use.
 * @param option The style options for the item.
 * @param index The model index of the item.
 */
void HoverRowDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                             const QModelIndex& index) const
{
    QStyleOptionViewItem opt(option);

    // Hover background (do not override selection background)
    if (index.row() == m_hovered_row && !(opt.state & QStyle::State_Selected))
    {
        if (auto* view = qobject_cast<TableView*>(const_cast<QWidget*>(opt.widget)))
        {
            painter->save();
            painter->fillRect(opt.rect, view->get_hover_row_color());
            painter->restore();
        }
    }

    // Prepare style option (fills opt_for_rect.text, font, etc.)
    QStyleOptionViewItem opt_for_rect(opt);
    initStyleOption(&opt_for_rect, index);

    // Get the text rect from the style (accounts for padding, decoration, etc.)
    QRect text_rect = opt.rect;
    const QStyle* style = opt.widget != nullptr ? opt.widget->style() : QApplication::style();
    text_rect = style->subElementRect(QStyle::SE_ItemViewItemText, &opt_for_rect, opt.widget);

    // Base painting first (selection, text, etc.)
    QStyledItemDelegate::paint(painter, opt, index);

    // Read highlight ranges from proxy (computed on-demand)
    const QVariant ranges_variant = index.data(LogSortFilterProxyModel::HighlightRangesRole);
    const QVariantList ranges_list = ranges_variant.toList();

    // Use the display text for highlight calculations
    const QString display_text = index.data(Qt::DisplayRole).toString();

    // Preconditions to paint highlights
    const bool has_ranges = !ranges_list.isEmpty();
    const bool has_text = !display_text.isEmpty();

    if (has_ranges && has_text)
    {
        const QFontMetrics fm(opt_for_rect.font);

        painter->save();
        painter->setClipRect(text_rect);

        // Translucent yellow overlay
        const QColor highlight_color(255, 255, 0, 140);

        // Calculate the actual text start position within text_rect
        const int text_margin =
            style->pixelMetric(QStyle::PM_FocusFrameHMargin, &opt_for_rect, opt.widget) + 1;
        const int text_start_x = text_rect.left() + text_margin;

        // Iterate and paint valid ranges
        for (const QVariant& item: ranges_list)
        {
            const QVariantMap m = item.toMap();
            const int start = m.value(QStringLiteral("start")).toInt();
            const int length = m.value(QStringLiteral("length")).toInt();

            const bool start_valid = (start >= 0) && (start < display_text.length());
            const bool length_valid = (length > 0);
            const bool range_valid = start_valid && length_valid;

            const int end =
                range_valid ? qMin(start + length, static_cast<int>(display_text.length())) : 0;

            // Compute geometry only if valid; otherwise values are ignored
            const int px_before = range_valid ? fm.horizontalAdvance(display_text.left(start)) : 0;
            const int px_match =
                range_valid ? fm.horizontalAdvance(display_text.mid(start, end - start)) : 0;

            const bool can_draw = range_valid && (px_match > 0);
            const int rect_x = can_draw ? (text_start_x + px_before) : 0;
            const int rect_w = can_draw ? px_match : 0;

            if (can_draw)
            {
                const QRect match_rect(rect_x, text_rect.top(), rect_w, text_rect.height());
                painter->fillRect(match_rect, highlight_color);
            }
        }

        painter->restore();
    }
}
