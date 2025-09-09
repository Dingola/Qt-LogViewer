#include "Qt-LogViewer/Views/Shared/HoverRowDelegate.h"

#include <QPainter>

#include "Qt-LogViewer/Views/Shared/TableView.h"

/**
 * @brief Constructs a HoverRowDelegate object.
 * @param parent The parent QObject, or nullptr.
 */
HoverRowDelegate::HoverRowDelegate(QObject* parent): QStyledItemDelegate(parent), m_hovered_row(-1)
{}

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
 * @brief Paints the item, applying the hover color from TableView if the row is hovered.
 *
 * If the row is currently hovered and not selected, the hover color from TableView is drawn
 * as the background. Otherwise, the default delegate painting is used.
 *
 * @param painter The painter to use.
 * @param option The style options for the item.
 * @param index The model index of the item.
 */
void HoverRowDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                             const QModelIndex& index) const
{
    QStyleOptionViewItem opt(option);

    // Use hover color from TableView if available
    if (index.row() == m_hovered_row && !(opt.state & QStyle::State_Selected))
    {
        if (auto* view = qobject_cast<TableView*>(const_cast<QWidget*>(opt.widget)))
        {
            painter->save();
            painter->fillRect(opt.rect, view->get_hover_row_color());
            painter->restore();
        }
    }

    QStyledItemDelegate::paint(painter, opt, index);
}
