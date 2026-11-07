/**
 * @file TwoLineItemDelegate.cpp
 * @brief Implements a configurable two-line item delegate.
 */

#include "Qt-LogViewer/Views/Shared/TwoLineItemDelegate.h"

#include <QApplication>
#include <QFontMetrics>
#include <QIcon>
#include <QPainter>
#include <QStyle>
#include <QStyleOptionViewItem>
#include <utility>

/**
 * @brief Constructs a delegate using the supplied semantic model roles.
 * @param roles Model roles used for each visual element.
 * @param parent Optional QObject parent.
 */
TwoLineItemDelegate::TwoLineItemDelegate(Roles roles, QObject* parent)
    : QStyledItemDelegate(parent), m_roles(roles)
{}

/**
 * @brief Replaces the role mapping used for subsequent paints.
 * @param roles New semantic model roles.
 */
auto TwoLineItemDelegate::set_roles(Roles roles) -> void
{
    m_roles = roles;
}

/**
 * @brief Replaces the visual layout configuration.
 * @param layout New spacing, sizing, and elision configuration.
 */
auto TwoLineItemDelegate::set_layout(Layout layout) -> void
{
    m_layout = layout;
}

/**
 * @brief Sets the conversion applied to metadata values.
 * @param formatter Formatter supplied by the host application.
 */
auto TwoLineItemDelegate::set_metadata_formatter(MetadataFormatter formatter) -> void
{
    m_metadata_formatter = std::move(formatter);
}

/**
 * @brief Paints the configured title, secondary text, metadata, icon, and
 * status badge.
 * @param painter Painter used for rendering.
 * @param option Current item style and geometry.
 * @param index Model index to render.
 */
auto TwoLineItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                                const QModelIndex& index) const -> void
{
    QStyleOptionViewItem opt(option);
    initStyleOption(&opt, index);

    painter->save();
    QStyle* style = opt.widget != nullptr ? opt.widget->style() : QApplication::style();
    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);

    const bool selected = opt.state.testFlag(QStyle::State_Selected);
    const bool enabled = opt.state.testFlag(QStyle::State_Enabled);
    const QPalette::ColorGroup color_group = enabled ? QPalette::Active : QPalette::Disabled;
    const QPalette::ColorRole text_role = selected ? QPalette::HighlightedText : QPalette::Text;
    const QColor text_color = opt.palette.color(color_group, text_role);
    QColor secondary_color(text_color);
    secondary_color.setAlphaF(selected ? 0.9 : 0.75);

    QRect content_rect = opt.rect.marginsRemoved(m_layout.content_margins);
    const QIcon icon = m_roles.icon >= 0 ? qvariant_cast<QIcon>(index.data(m_roles.icon)) : QIcon();
    if (!icon.isNull())
    {
        const int icon_top =
            content_rect.top() + (content_rect.height() - m_layout.icon_size.height()) / 2;
        const QRect icon_rect(content_rect.left(), icon_top, m_layout.icon_size.width(),
                              m_layout.icon_size.height());
        const QIcon::Mode icon_mode = enabled ? QIcon::Normal : QIcon::Disabled;
        icon.paint(painter, icon_rect, Qt::AlignCenter, icon_mode,
                   selected ? QIcon::On : QIcon::Off);
        content_rect.setLeft(icon_rect.right() + 1 + m_layout.icon_spacing);
    }

    QRect top_row = content_rect;
    const int available_rows_height = qMax(0, content_rect.height() - m_layout.row_spacing);
    top_row.setHeight(available_rows_height / 2);
    QRect bottom_row = content_rect;
    bottom_row.setTop(top_row.bottom() + 1 + m_layout.row_spacing);

    const QString title = m_roles.title >= 0 ? index.data(m_roles.title).toString() : QString();
    const QString secondary =
        m_roles.secondary >= 0 ? index.data(m_roles.secondary).toString() : QString();
    const QString metadata =
        m_roles.metadata >= 0 ? format_metadata(index.data(m_roles.metadata)) : QString();
    const QString status =
        m_roles.status >= 0 && !enabled ? index.data(m_roles.status).toString() : QString();

    QFont title_font = opt.font;
    title_font.setWeight(QFont::Medium);
    const QFontMetrics title_metrics(title_font);
    const int metadata_width =
        qMin(opt.fontMetrics.horizontalAdvance(metadata), qMax(0, top_row.width() / 2));
    const int metadata_reservation =
        metadata.isEmpty() ? 0 : metadata_width + m_layout.metadata_spacing;
    const QRect title_rect = top_row.adjusted(0, 0, -metadata_reservation, 0);

    painter->setFont(title_font);
    painter->setPen(text_color);
    painter->drawText(
        title_rect, Qt::AlignVCenter | Qt::AlignLeft | Qt::TextSingleLine,
        title_metrics.elidedText(title, m_layout.title_elide_mode, qMax(0, title_rect.width())));

    if (!metadata.isEmpty())
    {
        const QRect metadata_rect(top_row.right() - metadata_width + 1, top_row.top(),
                                  metadata_width, top_row.height());
        painter->setFont(opt.font);
        painter->setPen(secondary_color);
        painter->drawText(
            metadata_rect, Qt::AlignVCenter | Qt::AlignRight | Qt::TextSingleLine,
            opt.fontMetrics.elidedText(metadata, m_layout.metadata_elide_mode, metadata_width));
    }

    QRect secondary_rect = bottom_row;
    if (!status.isEmpty())
    {
        QFont status_font = opt.font;
        status_font.setWeight(QFont::Medium);
        const QFontMetrics status_metrics(status_font);
        const int desired_badge_width =
            status_metrics.horizontalAdvance(status) + (m_layout.badge_horizontal_padding * 2);
        const int badge_width = qMin(desired_badge_width, qMax(0, bottom_row.width() / 2));
        const QRect badge_rect(bottom_row.right() - badge_width + 1, bottom_row.top(), badge_width,
                               bottom_row.height());
        secondary_rect.setRight(badge_rect.left() - m_layout.badge_spacing);

        QColor badge_background = opt.palette.color(color_group, QPalette::Mid);
        badge_background.setAlphaF(selected ? 0.35 : 0.55);
        painter->setPen(Qt::NoPen);
        painter->setBrush(badge_background);
        painter->drawRoundedRect(badge_rect, m_layout.badge_radius, m_layout.badge_radius);
        painter->setFont(status_font);
        painter->setPen(opt.palette.color(color_group, QPalette::Text));
        painter->drawText(
            badge_rect.adjusted(m_layout.badge_horizontal_padding, 0,
                                -m_layout.badge_horizontal_padding, 0),
            Qt::AlignVCenter | Qt::AlignHCenter | Qt::TextSingleLine,
            status_metrics.elidedText(
                status, Qt::ElideRight,
                qMax(0, badge_rect.width() - (m_layout.badge_horizontal_padding * 2))));
    }

    painter->setFont(opt.font);
    painter->setPen(secondary_color);
    painter->drawText(secondary_rect, Qt::AlignVCenter | Qt::AlignLeft | Qt::TextSingleLine,
                      opt.fontMetrics.elidedText(secondary, m_layout.secondary_elide_mode,
                                                 qMax(0, secondary_rect.width())));
    painter->restore();
}

/**
 * @brief Computes the preferred two-line item size.
 * @param option Current item style and geometry.
 * @param index Model index being measured.
 * @return Preferred item size including configured margins and spacing.
 */
[[nodiscard]] auto TwoLineItemDelegate::sizeHint(const QStyleOptionViewItem& option,
                                                 const QModelIndex& index) const -> QSize
{
    Q_UNUSED(index);
    const int text_height = (option.fontMetrics.height() * 2) + m_layout.row_spacing;
    const int content_height = qMax(text_height, m_layout.icon_size.height());
    QSize result(option.rect.width(), content_height + m_layout.content_margins.top() +
                                          m_layout.content_margins.bottom());
    return result;
}

/**
 * @brief Formats a metadata model value for display.
 * @param value Raw metadata value.
 * @return Host-formatted metadata text or the value's string representation.
 */
[[nodiscard]] auto TwoLineItemDelegate::format_metadata(const QVariant& value) const -> QString
{
    QString result;
    if (m_metadata_formatter)
    {
        result = m_metadata_formatter(value);
    }
    else
    {
        result = value.toString();
    }
    return result;
}
