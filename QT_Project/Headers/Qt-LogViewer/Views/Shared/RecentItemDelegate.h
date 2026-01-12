#pragma once

#include <QDateTime>
#include <QStyledItemDelegate>

/**
 * @file RecentItemDelegate.h
 * @brief Declares the `RecentItemDelegate` used to render recent entries (files or sessions) in a
 * `QListView`.
 *
 * This delegate draws a two-line entry:
 * - Top row: bold title (left), formatted date (right)
 * - Bottom row: secondary text (elided in the middle)
 *
 * It is generic and reads values from three configurable roles:
 * - `title_role`     (QString)
 * - `secondary_role` (QString)
 * - `date_role`      (QDateTime)
 *
 * Styling (hover/selected backgrounds and borders) is driven by QSS. The delegate respects the
 * current palette and the `QStyle::PE_PanelItemViewItem` primitive so your QSS item states apply
 * cleanly.
 */
class RecentItemDelegate final: public QStyledItemDelegate
{
        Q_OBJECT

    public:
        /**
         * @brief Construct a delegate that reads item data from the given roles.
         * @param title_role Qt role id providing the title (e.g., file name or session name).
         * @param secondary_role Qt role id providing the secondary text (e.g., file path or session
         * id).
         * @param date_role Qt role id providing the last-opened date as `QDateTime`.
         * @param parent Optional QObject parent.
         */
        explicit RecentItemDelegate(int title_role, int secondary_role, int date_role,
                                    QObject* parent = nullptr);

        /**
         * @brief Paint the item using a consistent two-row layout.
         * @param painter Painter to draw with.
         * @param option Style option (includes state and rect).
         * @param index Model index to render.
         */
        auto paint(QPainter* painter, const QStyleOptionViewItem& option,
                   const QModelIndex& index) const -> void override;

        /**
         * @brief Compute a suitable size for the item.
         * @param option Style option (includes font metrics).
         * @param index Model index (unused).
         * @return Preferred item size with two text rows and padding.
         */
        [[nodiscard]] auto sizeHint(const QStyleOptionViewItem& option,
                                    const QModelIndex& index) const -> QSize override;

    private:
        /**
         * @brief Extract display texts (title, secondary, date) from the model index using
         * configured roles.
         * @param index Model index.
         * @param out_title Bold title text.
         * @param out_secondary Secondary text, elided when drawn.
         * @param out_date_text Right-aligned date text (formatted).
         */
        auto extract_texts(const QModelIndex& index, QString& out_title, QString& out_secondary,
                           QString& out_date_text) const -> void;

    private:
        int m_title_role;
        int m_secondary_role;
        int m_date_role;
};
