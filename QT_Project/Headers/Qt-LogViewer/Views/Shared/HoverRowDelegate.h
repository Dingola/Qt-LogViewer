#pragma once

#include <QStyledItemDelegate>

/**
 * @class HoverRowDelegate
 * @brief Custom item delegate to provide full-row hover highlighting in QTableView.
 *
 * This delegate allows the entire row to be highlighted on mouse hover.
 */
class HoverRowDelegate: public QStyledItemDelegate
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs a HoverRowDelegate object.
         * @param parent The parent QObject, or nullptr.
         */
        explicit HoverRowDelegate(QObject* parent = nullptr);

        /**
         * @brief Sets the currently hovered row.
         * @param row The row index to highlight, or -1 for none.
         */
        auto set_hovered_row(int row) -> void;

        /**
         * @brief Returns the currently hovered row.
         * @return The hovered row index, or -1 if none.
         */
        [[nodiscard]] auto hovered_row() const -> int;

        /**
         * @brief Paints the item, applying the hover color to the full row if applicable.
         * @param painter The painter to use.
         * @param option The style options for the item.
         * @param index The model index of the item.
         */
        void paint(QPainter* painter, const QStyleOptionViewItem& option,
                   const QModelIndex& index) const override;

    private:
        int m_hovered_row = -1;
};
