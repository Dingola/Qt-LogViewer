#include "Qt-LogViewer/Views/Shared/HoverRowDelegate.h"

#include <QApplication>
#include <QFontMetrics>
#include <QPainter>
#include <QStyle>

#include "Qt-LogViewer/Models/LogModel.h"
#include "Qt-LogViewer/Views/Shared/TableView.h"

/**
 * @file HoverRowDelegate.cpp
 * @brief Implements hovered-row painting and search-result highlighting.
 */

/**
 * @brief Constructs a HoverRowDelegate.
 * @param parent Parent object, or nullptr.
 */
HoverRowDelegate::HoverRowDelegate(QObject* parent): QStyledItemDelegate(parent) {}

/**
 * @brief Applies the table's severity foreground to the level column.
 * @param option Cell style option receiving the configured foreground.
 * @param index Model index providing the field identifier and log level.
 */
void HoverRowDelegate::initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const
{
    QStyledItemDelegate::initStyleOption(option, index);
    const auto* table = qobject_cast<const TableView*>(option->widget);
    const auto* model = qobject_cast<const LogModel*>(index.model());
    if (!(table == nullptr || model == nullptr ||
          model->get_column_field_id(index.column()) != LogField::Level))
    {
        const char* color_property = nullptr;
        switch (LogModel::map_log_level(index.data(LogModel::LevelRole).toString()))
        {
        case SimpleCppLogger::LogLevel::Trace:
            color_property = "trace_color";
            break;
        case SimpleCppLogger::LogLevel::Debug:
            color_property = "debug_color";
            break;
        case SimpleCppLogger::LogLevel::Info:
            color_property = "info_color";
            break;
        case SimpleCppLogger::LogLevel::Warning:
            color_property = "warning_color";
            break;
        case SimpleCppLogger::LogLevel::Error:
            color_property = "error_color";
            break;
        case SimpleCppLogger::LogLevel::Fatal:
            color_property = "fatal_color";
            break;
        default:
            break;
        }
        if (color_property != nullptr)
        {
            const QColor color = table->property(color_property).value<QColor>();
            if (color.isValid())
            {
                option->palette.setColor(QPalette::Text, color);
            }
        }
    }
}
/**
 * @brief Sets the currently hovered row.
 * @param row Row to highlight, or -1 when no row is hovered.
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
 * @return Hovered row, or -1 when no row is hovered.
 */
auto HoverRowDelegate::hovered_row() const -> int
{
    return m_hovered_row;
}

/**
 * @brief Sets the search used for highlighting displayed cell text.
 * @param text Plain search text or regular expression.
 * @param field Field whose displayed cells may be highlighted.
 * @param use_regex Whether text is interpreted as a regular expression.
 */
auto HoverRowDelegate::set_search_highlight(const QString& text, SearchField field,
                                            bool use_regex) -> void
{
    m_search_text = text;
    m_search_field = field;
    m_use_regex = use_regex;

    if (m_use_regex && !m_search_text.isEmpty())
    {
        m_search_regex =
            QRegularExpression(m_search_text, QRegularExpression::CaseInsensitiveOption);
    }
    else
    {
        m_search_regex = QRegularExpression();
    }
}

/**
 * @brief Finds the matching text ranges for a displayed table cell.
 * @param text Displayed cell text.
 * @param column Model column containing the text.
 * @return Start position and length of every matching range.
 */
auto HoverRowDelegate::get_highlight_ranges(const QString& text,
                                            int column) const -> QVector<QPair<int, int>>
{
    QVector<QPair<int, int>> ranges;
    bool search_column = false;
    QString field_id;
    const auto* table_view = qobject_cast<const TableView*>(parent());

    if (table_view != nullptr)
    {
        const auto* log_model = qobject_cast<const LogModel*>(table_view->model());

        if (log_model != nullptr)
        {
            field_id = log_model->get_column_field_id(column);
        }
    }

    if (field_id.isEmpty())
    {
        if (column == LogModel::Timestamp)
        {
            field_id = LogField::Timestamp;
        }
        else if (column == LogModel::Level)
        {
            field_id = LogField::Level;
        }
        else if (column == LogModel::Message)
        {
            field_id = LogField::Message;
        }
        else if (column == LogModel::AppName)
        {
            field_id = LogField::AppName;
        }
    }

    switch (m_search_field)
    {
    case SearchField::AllFields:
        search_column = true;
        break;

    case SearchField::Message:
        search_column = field_id == LogField::Message;
        break;

    case SearchField::Level:
        search_column = field_id == LogField::Level;
        break;

    case SearchField::AppName:
        search_column = field_id == LogField::AppName;
        break;

    case SearchField::Count:
        break;
    }

    const bool can_highlight = search_column && !m_search_text.isEmpty() && !text.isEmpty();

    if (can_highlight)
    {
        if (m_use_regex && m_search_regex.isValid())
        {
            QRegularExpressionMatchIterator matches = m_search_regex.globalMatch(text);

            while (matches.hasNext())
            {
                const QRegularExpressionMatch match = matches.next();
                const int start = match.capturedStart();
                const int length = match.capturedLength();

                if (start >= 0 && length > 0)
                {
                    ranges.append(qMakePair(start, length));
                }
            }
        }
        else if (!m_use_regex)
        {
            const int search_length = m_search_text.length();
            int position = 0;

            while (position < text.length())
            {
                const int found = text.indexOf(m_search_text, position, Qt::CaseInsensitive);

                if (found >= 0)
                {
                    ranges.append(qMakePair(found, search_length));
                    position = found + qMax(1, search_length);
                }
                else
                {
                    position = text.length();
                }
            }
        }
    }

    return ranges;
}
/**
 * @brief Paints the cell, hovered-row background and matching search ranges.
 * @param painter Painter used to render the cell.
 * @param option Style information for the cell.
 * @param index Model index represented by the cell.
 */
void HoverRowDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                             const QModelIndex& index) const
{
    QStyleOptionViewItem cell_option(option);

    const bool hovered = index.row() == m_hovered_row;

    const bool selected = cell_option.state.testFlag(QStyle::State_Selected);

    if (hovered && !selected)
    {
        const auto* table_view = qobject_cast<const TableView*>(cell_option.widget);

        if (table_view != nullptr)
        {
            painter->save();
            painter->fillRect(cell_option.rect, table_view->get_hover_row_color());
            painter->restore();
        }
    }

    QStyleOptionViewItem text_option(cell_option);
    initStyleOption(&text_option, index);

    const QStyle* style =
        cell_option.widget != nullptr ? cell_option.widget->style() : QApplication::style();

    const QRect text_rect =
        style->subElementRect(QStyle::SE_ItemViewItemText, &text_option, cell_option.widget);

    QStyledItemDelegate::paint(painter, cell_option, index);

    const QString display_text = index.data(Qt::DisplayRole).toString();

    const QVector<QPair<int, int>> ranges = get_highlight_ranges(display_text, index.column());

    const bool can_paint_highlights = !display_text.isEmpty() && !ranges.isEmpty();

    if (can_paint_highlights)
    {
        const QFontMetrics font_metrics(text_option.font);
        const auto* table_view = qobject_cast<const TableView*>(cell_option.widget);
        const QColor highlight_color = table_view != nullptr
                                           ? table_view->get_search_match_color()
                                           : text_option.palette.color(QPalette::Link);

        const int text_margin =
            style->pixelMetric(QStyle::PM_FocusFrameHMargin, &text_option, cell_option.widget) + 1;

        const int text_start_x = text_rect.left() + text_margin;

        painter->save();
        painter->setClipRect(cell_option.rect.adjusted(1, 1, -1, -1));

        for (const QPair<int, int>& range: ranges)
        {
            const int start = range.first;
            const int length = range.second;

            const bool start_valid = start >= 0 && start < display_text.length();

            const bool length_valid = length > 0;

            const bool range_valid = start_valid && length_valid;

            const int end =
                range_valid ? qMin(start + length, static_cast<int>(display_text.length())) : 0;

            const int pixels_before =
                range_valid ? font_metrics.horizontalAdvance(display_text.left(start)) : 0;

            const int match_width =
                range_valid ? font_metrics.horizontalAdvance(display_text.mid(start, end - start))
                            : 0;

            const bool can_draw = range_valid && match_width > 0;

            if (can_draw)
            {
                const int match_top =
                    text_rect.top() + (text_rect.height() - font_metrics.height()) / 2;
                const QRect match_rect(text_start_x + pixels_before, match_top, match_width,
                                       font_metrics.height());
                // Add breathing room without changing text layout or row height.
                const QRect marker_rect = match_rect.adjusted(-2, -2, 2, 2)
                                              .intersected(cell_option.rect.adjusted(1, 1, -1, -1));
                QColor marker_fill(highlight_color);
                marker_fill.setAlpha(55);
                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->setPen(QPen(highlight_color, 1.0));
                painter->setBrush(marker_fill);
                painter->drawRoundedRect(QRectF(marker_rect).adjusted(0.5, 0.5, -0.5, -0.5), 2.0,
                                         2.0);

                // Restore the normal text color above the translucent fill.
                // Drawing the entire string under a clip keeps its original spacing.
                painter->save();
                painter->setClipRect(match_rect, Qt::IntersectClip);
                painter->setFont(text_option.font);
                painter->setPen(text_option.palette.color(selected ? QPalette::HighlightedText
                                                                   : QPalette::Text));
                painter->drawText(QRect(text_start_x, text_rect.top(),
                                        text_rect.width() - 2 * text_margin, text_rect.height()),
                                  Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine,
                                  font_metrics.elidedText(display_text, text_option.textElideMode,
                                                          text_rect.width() - 2 * text_margin));
                painter->restore();
            }
        }

        painter->restore();
    }
}
