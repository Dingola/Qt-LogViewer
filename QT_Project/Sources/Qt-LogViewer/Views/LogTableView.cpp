#include "Qt-LogViewer/Views/LogTableView.h"

#include <QAbstractItemView>
#include <QHeaderView>

/**
 * @brief Constructs a LogTableView object.
 *
 * Sets up default selection, sorting, and mouse tracking for log display.
 *
 * @param parent The parent widget, or nullptr.
 */
LogTableView::LogTableView(QWidget* parent): TableView(parent)
{
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);

    auto hover_delegate = new HoverRowDelegate(this);
    setItemDelegate(hover_delegate);
    setMouseTracking(true);
    connect(this, &LogTableView::hover_index_changed, hover_delegate,
            [hover_delegate](const QModelIndex& index) {
                hover_delegate->set_hovered_row(index.isValid() ? index.row() : -1);
                if (auto* view = qobject_cast<QAbstractItemView*>(hover_delegate->parent()))
                {
                    view->viewport()->update();
                }
            });
}

/**
 * @brief Automatically resizes the columns based on the current viewport size.
 *
 * This method sets the column widths proportionally for the log table.
 * Call this after the model is set or when the parent window is resized.
 */
auto LogTableView::auto_resize_columns() -> void
{
    bool valid_model = model() && model()->columnCount() >= 4;

    if (valid_model)
    {
        int total_width = viewport()->width();
        int col_0_width = static_cast<int>(total_width * 0.15);
        int col_1_width = static_cast<int>(total_width * 0.10);
        int col_2_width = static_cast<int>(total_width * 0.50);
        int col_3_width = static_cast<int>(total_width * 0.15);

        setColumnWidth(LogModel::Timestamp, col_0_width);
        setColumnWidth(LogModel::Level, col_1_width);
        setColumnWidth(LogModel::Message, col_2_width);
        setColumnWidth(LogModel::AppName, col_3_width);
#ifdef QT_DEBUG_VERBOSE
        qDebug() << "Resizing columns to widths:" << col_0_width << col_1_width << col_2_width
                 << col_3_width;
#endif
    }
    else
    {
        qWarning() << "Table model invalid or has too few columns!";
    }
}

/**
 * @brief Sets the model for the table view.
 * @param model The model to set (should be a LogModel or compatible).
 */
void LogTableView::setModel(QAbstractItemModel* model)
{
    TableView::setModel(model);

    auto* header = horizontalHeader();
    int column_count = this->model()->columnCount();

    for (int i = 0; i < column_count; ++i)
    {
        header->setSectionResizeMode(
            i, ((i == (column_count - 1)) ? QHeaderView::Stretch : QHeaderView::Interactive));
    }
}
