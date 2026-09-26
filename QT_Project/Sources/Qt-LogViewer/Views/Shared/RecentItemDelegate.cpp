/**
 * @file RecentItemDelegate.cpp
 * @brief Implements the `RecentItemDelegate` used to render recent entries (files or sessions).
 *
 * The delegate reads three roles (title, secondary, date) provided at construction and
 * renders them in a two-row layout. Visual selection/hover behavior is controlled via QSS.
 */

#include "Qt-LogViewer/Views/Shared/RecentItemDelegate.h"

#include <QFontMetrics>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionViewItem>

/**
 * @brief Construct a delegate with role configuration.
 * @param title_role Role id for the title string.
 * @param secondary_role Role id for the secondary string.
 * @param date_role Role id for the `QDateTime` value.
 * @param parent Optional QObject parent.
 */
RecentItemDelegate::RecentItemDelegate(int title_role, int secondary_role, int date_role,
                                       QObject* parent)
    : QStyledItemDelegate(parent),
      m_title_role(title_role),
      m_secondary_role(secondary_role),
      m_date_role(date_role)
{}

/**
 * @brief Paints the two-row entry with title/date on the top and secondary text on the bottom.
 * @param painter Painter to draw with.
 * @param option Style option (includes selection/hover state).
 * @param index Model index to render.
 */
auto RecentItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                               const QModelIndex& index) const -> void
{
    QStyleOptionViewItem opt(option);
    initStyleOption(&opt, index);

    painter->save();

    // Respect QSS states (hover/selected) by drawing the base panel via style primitive.
    if (opt.widget != nullptr)
    {
        opt.widget->style()->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);
    }

    const QPalette& pal = opt.palette;
    const QRect content_rect = opt.rect.adjusted(12, 8, -12, -8);

    QString title;
    QString secondary;
    QString date_text;
    extract_texts(index, title, secondary, date_text);

    // Keep metadata readable; separate it by tone rather than tiny type.
    QFont title_font = opt.font;
    title_font.setWeight(QFont::Medium);

    QFont secondary_font = opt.font;
    const bool selected = opt.state.testFlag(QStyle::State_Selected);
    const QColor text_color = pal.color(selected ? QPalette::HighlightedText : QPalette::Text);
    QColor secondary_color(text_color);
    secondary_color.setAlphaF(selected ? 0.9 : 0.75);

    // Layout rows
    QRect top_row = content_rect;
    top_row.setHeight(content_rect.height() / 2);

    QRect bottom_row = content_rect;
    bottom_row.setTop(top_row.bottom() + 2);

    // Title left
    painter->setFont(title_font);
    painter->setPen(text_color);
    const int date_width =
        qMin(opt.fontMetrics.horizontalAdvance(date_text), qMax(0, top_row.width() / 2));
    const QRect title_rect = top_row.adjusted(0, 0, -date_width - 16, 0);
    painter->drawText(
        title_rect, Qt::AlignVCenter | Qt::AlignLeft | Qt::TextSingleLine,
        QFontMetrics(title_font).elidedText(title, Qt::ElideRight, qMax(0, title_rect.width())));

    // Date right
    painter->setFont(opt.font);
    painter->setPen(secondary_color);
    const QRect date_rect(top_row.right() - date_width + 1, top_row.top(), date_width,
                          top_row.height());
    painter->drawText(date_rect, Qt::AlignVCenter | Qt::AlignRight | Qt::TextSingleLine,
                      opt.fontMetrics.elidedText(date_text, Qt::ElideRight, date_width));

    // Secondary (elided)
    painter->setFont(secondary_font);
    painter->setPen(secondary_color);
    const QString elided_secondary =
        opt.fontMetrics.elidedText(secondary, Qt::ElideMiddle, bottom_row.width());
    painter->drawText(bottom_row, Qt::AlignVCenter | Qt::AlignLeft, elided_secondary);

    painter->restore();
}

/**
 * @brief Returns the preferred size: two text rows plus vertical padding.
 * @param option Style option (for font metrics).
 * @param index Model index (unused).
 * @return Size with height for two lines and margins.
 */
[[nodiscard]] auto RecentItemDelegate::sizeHint(const QStyleOptionViewItem& option,
                                                const QModelIndex& index) const -> QSize
{
    Q_UNUSED(index);
    QSize s(option.rect.width(), 0);
    const int line_height = option.fontMetrics.height();
    const int height = (line_height * 2) + 16;
    s.setHeight(height);
    return s;
}

/**
 * @brief Reads title, secondary and date text from the configured roles.
 * @param index Model index to read from.
 * @param out_title Title string.
 * @param out_secondary Secondary string.
 * @param out_date_text Formatted date string ("yyyy-MM-dd HH:mm") if valid.
 */
auto RecentItemDelegate::extract_texts(const QModelIndex& index, QString& out_title,
                                       QString& out_secondary, QString& out_date_text) const -> void
{
    out_title = index.data(m_title_role).toString();
    out_secondary = index.data(m_secondary_role).toString();

    const QDateTime dt = index.data(m_date_role).toDateTime();
    if (dt.isValid())
    {
        out_date_text = dt.toString(QStringLiteral("yyyy-MM-dd HH:mm"));
    }
    else
    {
        out_date_text.clear();
    }
}
