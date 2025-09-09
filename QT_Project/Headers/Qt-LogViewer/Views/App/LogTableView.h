#pragma once

#include "Qt-LogViewer/Models/LogModel.h"
#include "Qt-LogViewer/Views/Shared/HoverRowDelegate.h"
#include "Qt-LogViewer/Views/Shared/TableView.h"

/**
 * @class LogTableView
 * @brief Specialized TableView for displaying log entries in the LogViewer.
 *
 * This class extends TableView and provides default configuration and hooks
 * for log-specific features (selection, sorting, mouse tracking, etc.).
 * Use this class wherever a log entry table is required in the application.
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
};
