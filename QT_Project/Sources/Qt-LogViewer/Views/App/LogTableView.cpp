#include "Qt-LogViewer/Views/App/LogTableView.h"

#include <QAbstractItemView>
#include <QHeaderView>

/**
 * @file LogTableView.cpp
 * @brief Implements log-table configuration and search highlighting.
 */

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

    m_hover_delegate = new HoverRowDelegate(this);
    setItemDelegate(m_hover_delegate);
    setMouseTracking(true);
    connect(this, &LogTableView::hover_index_changed, m_hover_delegate,
            [this](const QModelIndex& index) {
                m_hover_delegate->set_hovered_row(index.isValid() ? index.row() : -1);

                viewport()->update();
            });

    setSortingEnabled(true);
}

/**
 * @brief Automatically resizes the columns based on the current viewport size.
 *
 * This method sets the column widths proportionally for the log table.
 * Call this after the model is set or when the parent window is resized.
 */
auto LogTableView::auto_resize_columns() -> void
{
    const auto* log_model = qobject_cast<const LogModel*>(model());
    const bool valid_model = log_model != nullptr && log_model->columnCount() > 0;

    if (valid_model)
    {
        const int total_width = viewport()->width();
        QVector<double> weights;
        double total_weight = 0.0;

        for (int column = 0; column < log_model->columnCount(); ++column)
        {
            const QString field_id = log_model->get_column_field_id(column);
            double weight = 1.5;

            if (field_id == LogField::Timestamp)
            {
                weight = 2.0;
            }
            else if (field_id == LogField::Level)
            {
                weight = 1.0;
            }
            else if (field_id == LogField::Message)
            {
                weight = 5.0;
            }
            else if (field_id == LogField::AppName)
            {
                weight = 1.5;
            }

            weights.append(weight);
            total_weight += weight;
        }

        for (int column = 0; column < weights.size(); ++column)
        {
            setColumnWidth(column,
                           static_cast<int>(total_width * weights.at(column) / total_weight));
        }
#ifdef QT_DEBUG_VERBOSE
        qDebug() << "Resized" << weights.size() << "dynamic log columns";
#endif
    }
    else
    {
        qWarning() << "Table model invalid or has no columns!";
    }
}

/**
 * @brief Sets the model for the table view.
 * @param model The model to set (should be a LogModel or compatible).
 */
void LogTableView::setModel(QAbstractItemModel* model)
{
    if (this->model() != nullptr)
    {
        disconnect(this->model(), nullptr, this, nullptr);
    }

    TableView::setModel(model);

    const auto configure_header = [this]() {
        auto* header = horizontalHeader();
        const int column_count = this->model() != nullptr ? this->model()->columnCount() : 0;

        for (int i = 0; i < column_count; ++i)
        {
            header->setSectionResizeMode(
                i, ((i == (column_count - 1)) ? QHeaderView::Stretch : QHeaderView::Interactive));
        }
    };

    configure_header();

    if (model != nullptr)
    {
        connect(model, &QAbstractItemModel::columnsInserted, this,
                [configure_header]() { configure_header(); });
        connect(model, &QAbstractItemModel::modelReset, this,
                [configure_header]() { configure_header(); });
    }
}

/**
 * @brief Sets the search used for highlighting table cells.
 * @param text Search text or regular expression.
 * @param field Field whose cells are highlighted.
 * @param use_regex Whether text is interpreted as a regular expression.
 */
auto LogTableView::set_search_highlight(const QString& text, SearchField field,
                                        bool use_regex) -> void
{
    if (m_hover_delegate != nullptr)
    {
        m_hover_delegate->set_search_highlight(text, field, use_regex);
        viewport()->update();
    }
}
