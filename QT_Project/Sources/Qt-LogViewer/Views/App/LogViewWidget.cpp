#include "Qt-LogViewer/Views/App/LogViewWidget.h"

#include <QItemSelectionModel>
#include <QLayout>

#include "Qt-LogViewer/Views/App/LogFilterWidget.h"
#include "Qt-LogViewer/Views/App/LogTableView.h"
#include "ui_LogViewWidget.h"

/**
 * @brief Constructs a LogViewWidget object.
 *
 * Initializes UI components and connects internal signals to be forwarded outward.
 *
 * @param parent The parent widget, or nullptr.
 */
LogViewWidget::LogViewWidget(QWidget* parent)
    : QWidget(parent), ui(new Ui::LogViewWidget), m_view_id(QUuid())
{
    ui->setupUi(this);
    setAttribute(Qt::WA_StyledBackground, true);
    ui->verticalLayout->setContentsMargins(9, 0, 9, 0);

    // Forward filter signals
    connect(ui->logFilterWidget, &LogFilterWidget::app_filter_changed, this,
            &LogViewWidget::app_filter_changed);
    connect(ui->logFilterWidget, &LogFilterWidget::log_level_filter_changed, this,
            &LogViewWidget::log_level_filter_changed);

    // Forward current row changes in the table view
    QItemSelectionModel* selection_model = ui->logTableView->selectionModel();

    if (selection_model != nullptr)
    {
        connect(selection_model, &QItemSelectionModel::currentRowChanged, this,
                &LogViewWidget::current_row_changed);
    }
}

/**
 * @brief Destroys the LogViewWidget object.
 *
 * Cleans up allocated UI resources.
 */
LogViewWidget::~LogViewWidget()
{
    delete ui;
}

/**
 * @brief Sets the unique view id associated with this widget.
 *
 * Emits `view_id_changed` if the id changes.
 *
 * @param view_id The new view id.
 */
auto LogViewWidget::set_view_id(const QUuid& view_id) -> void
{
    bool changed = (m_view_id != view_id);
    m_view_id = view_id;

    if (changed)
    {
        emit view_id_changed(m_view_id);
    }
}

/**
 * @brief Gets the unique view id associated with this widget.
 *
 * @return The current view id (may be null QUuid if not set).
 */
auto LogViewWidget::get_view_id() const -> QUuid
{
    QUuid value = m_view_id;
    return value;
}

/**
 * @brief Sets the model on the internal log table view.
 *
 * Reconnects the selection model's currentRowChanged signal.
 *
 * @param model The model to assign.
 */
auto LogViewWidget::set_model(QAbstractItemModel* model) -> void
{
    ui->logTableView->setModel(model);
    QItemSelectionModel* selection_model = ui->logTableView->selectionModel();

    if (selection_model != nullptr)
    {
        connect(selection_model, &QItemSelectionModel::currentRowChanged, this,
                &LogViewWidget::current_row_changed);
    }
}

/**
 * @brief Convenience method: resizes columns in the internal table view.
 */
auto LogViewWidget::auto_resize_columns() -> void
{
    ui->logTableView->auto_resize_columns();
}

/**
 * @brief Sets the application names for the filter widget.
 * @param app_names Set of application names.
 */
auto LogViewWidget::set_app_names(const QSet<QString>& app_names) -> void
{
    ui->logFilterWidget->set_app_names(app_names);
}

/**
 * @brief Selects a specific application name in the filter widget if present.
 * @param app_name Application name to select.
 */
auto LogViewWidget::set_current_app_name_filter(const QString& app_name) -> void
{
    ui->logFilterWidget->set_current_app_name_filter(app_name);
}

/**
 * @brief Sets available log level names for creating level items.
 * @param log_levels List of log level names.
 */
auto LogViewWidget::set_available_log_levels(const QVector<QString>& log_levels) -> void
{
    ui->logFilterWidget->set_available_log_levels(log_levels);
}

/**
 * @brief Sets which log levels are currently checked.
 * @param levels Set of log level names to check.
 */
auto LogViewWidget::set_log_levels(const QSet<QString>& levels) -> void
{
    ui->logFilterWidget->set_log_levels(levels);
}

/**
 * @brief Updates per-log-level counts displayed in the filter items.
 * @param level_counts Map of level name to count.
 */
auto LogViewWidget::set_log_level_counts(const QMap<QString, int>& level_counts) -> void
{
    ui->logFilterWidget->set_log_level_counts(level_counts);
}

/**
 * @brief Shows or hides the filter widget area.
 *
 * @param visible True to show, false to hide.
 */
auto LogViewWidget::set_filter_widget_visible(bool visible) -> void
{
    ui->logFilterWidget->setVisible(visible);
}

/**
 * @brief Resets all filter related UI elements (apps + levels).
 *
 * Clears application names, log levels, counts.
 */
auto LogViewWidget::clear_filters() -> void
{
    ui->logFilterWidget->set_app_names({});
    ui->logFilterWidget->set_available_log_levels({});
    ui->logFilterWidget->set_log_levels({});
    ui->logFilterWidget->set_log_level_counts({});
}

/**
 * @brief Returns the currently selected application name.
 * @return Current application name.
 */
auto LogViewWidget::get_current_app_name() const -> QString
{
    QString value = ui->logFilterWidget->get_current_app_name();
    return value;
}

/**
 * @brief Returns the set of currently selected log levels.
 * @return Set of selected log level names.
 */
auto LogViewWidget::get_current_log_levels() const -> QSet<QString>
{
    QSet<QString> levels = ui->logFilterWidget->get_current_log_levels();
    return levels;
}

/**
 * @brief Returns a pointer to the internal LogFilterWidget.
 * @return Pointer to LogFilterWidget.
 */
auto LogViewWidget::get_filter_widget() const -> LogFilterWidget*
{
    LogFilterWidget* widget = ui->logFilterWidget;
    return widget;
}

/**
 * @brief Returns a pointer to the internal LogTableView.
 * @return Pointer to LogTableView.
 */
auto LogViewWidget::get_table_view() const -> LogTableView*
{
    LogTableView* widget = ui->logTableView;
    return widget;
}

/**
 * @brief Handles language change and other UI change events.
 * @param event The change event.
 */
auto LogViewWidget::changeEvent(QEvent* event) -> void
{
    if (event != nullptr && event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }

    QWidget::changeEvent(event);
}
