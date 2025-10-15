#include "Qt-LogViewer/Views/App/LogFilterBarWidget.h"

#include "Qt-LogViewer/Views/App/LogFilterWidget.h"
#include "Qt-LogViewer/Views/App/SearchBarWidget.h"
#include "ui_LogFilterBarWidget.h"

/**
 * @brief Constructs a LogFilterBarWidget object.
 *
 * Initializes the filter bar widget and its UI.
 *
 * @param parent The parent widget, or nullptr if this is a top-level widget.
 */
LogFilterBarWidget::LogFilterBarWidget(QWidget* parent)
    : QWidget(parent), ui(new Ui::LogFilterBarWidget)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_StyledBackground, true);

    // Forward signals from LogFilterWidget
    connect(ui->logFilterWidget, &LogFilterWidget::app_filter_changed, this,
            &LogFilterBarWidget::app_filter_changed);
    connect(ui->logFilterWidget, &LogFilterWidget::log_level_filter_changed, this,
            &LogFilterBarWidget::log_level_filter_changed);

    // Forward signals from SearchBarWidget
    connect(ui->searchBarWidget, &SearchBarWidget::search_requested, this,
            &LogFilterBarWidget::search_requested);
    connect(ui->searchBarWidget, &SearchBarWidget::search_text_changed, this,
            &LogFilterBarWidget::search_text_changed);
    connect(ui->searchBarWidget, &SearchBarWidget::search_field_changed, this,
            &LogFilterBarWidget::search_field_changed);
    connect(ui->searchBarWidget, &SearchBarWidget::regex_toggled, this,
            &LogFilterBarWidget::regex_toggled);
}

/**
 * @brief Destroys the LogFilterBarWidget object.
 *
 * Cleans up any resources used by the filter bar widget.
 */
LogFilterBarWidget::~LogFilterBarWidget()
{
    delete ui;
}

/**
 * @brief Sets the application names in the combo box.
 *
 * This overload sets the application names without a default entry.
 *
 * @param app_names The set of application names to populate the combo box.
 */
auto LogFilterBarWidget::set_app_names(const QSet<QString>& app_names) -> void
{
    ui->logFilterWidget->set_app_names(app_names);
}

/**
 * @brief Sets the currently selected application name in the combo box.
 *
 * If the provided application name does not exist in the combo box, no selection is made.
 *
 * @param app_name The application name to select.
 */
auto LogFilterBarWidget::set_current_app_name_filter(const QString& app_name) -> void
{
    ui->logFilterWidget->set_current_app_name_filter(app_name);
}

/**
 * @brief Sets the available log levels in the filter widget.
 * @param log_levels List of log level names (e.g., "TRACE", "DEBUG").
 */
auto LogFilterBarWidget::set_available_log_levels(const QVector<QString>& log_levels) -> void
{
    ui->logFilterWidget->set_available_log_levels(log_levels);
}

/**
 * @brief Sets the checked state of log level checkboxes in the filter widget.
 *
 * @param levels The set of log levels to check.
 */
auto LogFilterBarWidget::set_log_levels(const QSet<QString>& levels) -> void
{
    ui->logFilterWidget->set_log_levels(levels);
}

/**
 * @brief Sets the count for each log level in the filter widget.
 * @param level_counts Map of log level name to count.
 */
auto LogFilterBarWidget::set_log_level_counts(const QMap<QString, int>& level_counts) -> void
{
    ui->logFilterWidget->set_log_level_counts(level_counts);
}

/**
 * @brief Sets the available search fields in the search bar widget.
 *
 * @param fields The list of search fields.
 */
auto LogFilterBarWidget::set_search_fields(const QStringList& fields) -> void
{
    ui->searchBarWidget->set_search_fields(fields);
}

/**
 * @brief Sets the placeholder text for the search line edit in the search bar widget.
 *
 * @param text The placeholder text.
 */
auto LogFilterBarWidget::set_search_placeholder(const QString& text) -> void
{
    ui->searchBarWidget->set_search_placeholder(text);
}

/**
 * @brief Returns the currently selected application name.
 *
 * @return The selected application name.
 */
auto LogFilterBarWidget::get_current_app_name() const -> QString
{
    return ui->logFilterWidget->get_current_app_name();
}

/**
 * @brief Returns the currently selected log levels.
 *
 * @return The set of selected log levels.
 */
auto LogFilterBarWidget::get_current_log_levels() const -> QSet<QString>
{
    return ui->logFilterWidget->get_current_log_levels();
}

/**
 * @brief Returns the current search text.
 *
 * @return The search text.
 */
auto LogFilterBarWidget::get_search_text() const -> QString
{
    return ui->searchBarWidget->get_search_text();
}

/**
 * @brief Returns the currently selected search field.
 *
 * @return The search field.
 */
auto LogFilterBarWidget::get_search_field() const -> QString
{
    return ui->searchBarWidget->get_search_field();
}

/**
 * @brief Returns whether regex is enabled.
 *
 * @return True if regex is enabled, false otherwise.
 */
auto LogFilterBarWidget::get_use_regex() const -> bool
{
    return ui->searchBarWidget->get_use_regex();
}

/**
 * @brief Returns a pointer to the contained LogFilterWidget.
 *
 * @return Pointer to LogFilterWidget.
 */
auto LogFilterBarWidget::get_filter_widget() const -> LogFilterWidget*
{
    return ui->logFilterWidget;
}

/**
 * @brief Returns a pointer to the contained SearchBarWidget.
 *
 * @return Pointer to SearchBarWidget.
 */
auto LogFilterBarWidget::get_search_bar_widget() const -> SearchBarWidget*
{
    return ui->searchBarWidget;
}

/**
 * @brief Handles change events to update the UI.
 * @param event The change event.
 */
auto LogFilterBarWidget::changeEvent(QEvent* event) -> void
{
    if (event != nullptr && event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }

    QWidget::changeEvent(event);
}
