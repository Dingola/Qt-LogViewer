#pragma once

#include <QPair>
#include <QRegularExpression>
#include <QString>
#include <QStyledItemDelegate>
#include <QVector>

#include "Qt-LogViewer/Models/SearchFields.h"

/**
 * @file HoverRowDelegate.h
 * @brief Declares a table delegate for row hovering and search-result highlighting.
 */

/**
 * @class HoverRowDelegate
 * @brief Paints hovered rows and highlights matching text inside table cells.
 *
 * The delegate stores the current presentation-only search configuration. It does
 * not filter rows; filtering remains the responsibility of the database query.
 */
class HoverRowDelegate: public QStyledItemDelegate
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs a HoverRowDelegate.
         * @param parent Parent object, or nullptr.
         */
        explicit HoverRowDelegate(QObject* parent = nullptr);

        /**
         * @brief Sets the currently hovered row.
         * @param row Row to highlight, or -1 when no row is hovered.
         */
        auto set_hovered_row(int row) -> void;

        /**
         * @brief Returns the currently hovered row.
         * @return Hovered row, or -1 when no row is hovered.
         */
        [[nodiscard]] auto hovered_row() const -> int;

        /**
         * @brief Sets the search used for highlighting displayed cell text.
         * @param text Plain search text or regular expression.
         * @param field Field whose displayed cells may be highlighted.
         * @param use_regex Whether text is interpreted as a regular expression.
         */
        auto set_search_highlight(const QString& text, SearchField field, bool use_regex) -> void;

        /**
         * @brief Finds the matching text ranges for a displayed table cell.
         * @param text Displayed cell text.
         * @param column Model column containing the text.
         * @return Start position and length of every matching range.
         */
        [[nodiscard]] auto get_highlight_ranges(const QString& text,
                                                int column) const -> QVector<QPair<int, int>>;

        /**
         * @brief Paints the cell, hovered-row background and matching search ranges.
         * @param painter Painter used to render the cell.
         * @param option Style information for the cell.
         * @param index Model index represented by the cell.
         */
        void paint(QPainter* painter, const QStyleOptionViewItem& option,
                   const QModelIndex& index) const override;

    private:
        int m_hovered_row{-1};
        QString m_search_text;
        SearchField m_search_field{SearchField::AllFields};
        bool m_use_regex{false};
        QRegularExpression m_search_regex;
};
