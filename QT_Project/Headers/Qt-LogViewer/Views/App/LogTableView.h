#pragma once

#include "Qt-LogViewer/Models/LogModel.h"
#include "Qt-LogViewer/Views/Shared/HoverRowDelegate.h"
#include "Qt-LogViewer/Views/Shared/TableView.h"

/**
 * @file LogTableView.h
 * @brief Declares the log table with hover and search-highlight presentation.
 */

/**
 * @class LogTableView
 * @brief Displays database-backed log pages with sorting, hovering and search highlighting.
 *
 * The table owns a HoverRowDelegate and forwards presentation-only search
 * settings to it. Filtering and pagination are handled outside the table.
 */
class LogTableView: public TableView
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs a LogTableView object.
         *
         * Sets up default selection, sorting, and mouse tracking for log display.
         *
         * @param parent The parent widget, or nullptr.
         */
        explicit LogTableView(QWidget* parent = nullptr);

        /**
         * @brief Destroys the LogTableView object.
         */
        ~LogTableView() override = default;

        /**
         * @brief Automatically resizes the columns based on the current viewport size.
         *
         * This method sets the column widths proportionally for the log table.
         * Call this after the model is set or when the parent window is resized.
         */
        auto auto_resize_columns() -> void;

        /**
         * @brief Sets the model for the table view.
         * @param model The model to set (should be a LogModel or compatible).
         */
        void setModel(QAbstractItemModel* model) override;

        /**
         * @brief Sets the search used for highlighting table cells.
         * @param text Search text or regular expression.
         * @param field Field whose cells are highlighted.
         * @param use_regex Whether text is interpreted as a regular expression.
         */
        auto set_search_highlight(const QString& text, SearchField field, bool use_regex) -> void;

    private:
        HoverRowDelegate* m_hover_delegate{nullptr};
};
