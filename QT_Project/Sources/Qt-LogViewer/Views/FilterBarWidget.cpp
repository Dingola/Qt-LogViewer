#include "Qt-LogViewer/Views/FilterBarWidget.h"

#include "Qt-LogViewer/Views/FilterWidget.h"
#include "Qt-LogViewer/Views/SearchBarWidget.h"
#include "ui_FilterBarWidget.h"

/**
 * @brief Constructs a FilterBarWidget object.
 *
 * Initializes the filter bar widget and its UI.
 *
 * @param parent The parent widget, or nullptr if this is a top-level widget.
 */
FilterBarWidget::FilterBarWidget(QWidget* parent): QWidget(parent), ui(new Ui::FilterBarWidget)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_StyledBackground, true);

    // Forward signals from FilterWidget
    connect(ui->filterWidget, &FilterWidget::app_filter_changed, this,
            &FilterBarWidget::app_filter_changed);
    connect(ui->filterWidget, &FilterWidget::log_level_filter_changed, this,
            &FilterBarWidget::log_level_filter_changed);

    // Forward signals from SearchBarWidget
    connect(ui->searchBarWidget, &SearchBarWidget::search_requested, this,
            &FilterBarWidget::search_requested);
    connect(ui->searchBarWidget, &SearchBarWidget::search_text_changed, this,
            &FilterBarWidget::search_text_changed);
    connect(ui->searchBarWidget, &SearchBarWidget::search_field_changed, this,
            &FilterBarWidget::search_field_changed);
    connect(ui->searchBarWidget, &SearchBarWidget::regex_toggled, this,
            &FilterBarWidget::regex_toggled);
}

/**
 * @brief Destroys the FilterBarWidget object.
 *
 * Cleans up any resources used by the filter bar widget.
 */
FilterBarWidget::~FilterBarWidget()
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
auto FilterBarWidget::set_app_names(const QSet<QString>& app_names) -> void
{
    ui->filterWidget->set_app_names(app_names);
}

/**
 * @brief Sets the checked state of log level checkboxes in the filter widget.
 *
 * @param levels The set of log levels to check.
 */
auto FilterBarWidget::set_log_levels(const QSet<QString>& levels) -> void
{
    ui->filterWidget->set_log_levels(levels);
}

/**
 * @brief Sets the available search fields in the search bar widget.
 *
 * @param fields The list of search fields.
 */
auto FilterBarWidget::set_search_fields(const QStringList& fields) -> void
{
    ui->searchBarWidget->set_search_fields(fields);
}

/**
 * @brief Sets the placeholder text for the search line edit in the search bar widget.
 *
 * @param text The placeholder text.
 */
auto FilterBarWidget::set_search_placeholder(const QString& text) -> void
{
    ui->searchBarWidget->set_search_placeholder(text);
}

/**
 * @brief Returns the currently selected application name.
 *
 * @return The selected application name.
 */
auto FilterBarWidget::get_current_app_name() const -> QString
{
    return ui->filterWidget->get_current_app_name();
}

/**
 * @brief Returns the currently selected log levels.
 *
 * @return The set of selected log levels.
 */
auto FilterBarWidget::get_current_log_levels() const -> QSet<QString>
{
    return ui->filterWidget->get_current_log_levels();
}

/**
 * @brief Returns the current search text.
 *
 * @return The search text.
 */
auto FilterBarWidget::get_search_text() const -> QString
{
    return ui->searchBarWidget->get_search_text();
}

/**
 * @brief Returns the currently selected search field.
 *
 * @return The search field.
 */
auto FilterBarWidget::get_search_field() const -> QString
{
    return ui->searchBarWidget->get_search_field();
}

/**
 * @brief Returns whether regex is enabled.
 *
 * @return True if regex is enabled, false otherwise.
 */
auto FilterBarWidget::get_use_regex() const -> bool
{
    return ui->searchBarWidget->get_use_regex();
}

/**
 * @brief Returns a pointer to the contained FilterWidget.
 *
 * @return Pointer to FilterWidget.
 */
auto FilterBarWidget::get_filter_widget() const -> FilterWidget*
{
    return ui->filterWidget;
}

/**
 * @brief Returns a pointer to the contained SearchBarWidget.
 *
 * @return Pointer to SearchBarWidget.
 */
auto FilterBarWidget::get_search_bar_widget() const -> SearchBarWidget*
{
    return ui->searchBarWidget;
}

/**
 * @brief Handles change events to update the UI.
 * @param event The change event.
 */
auto FilterBarWidget::changeEvent(QEvent* event) -> void
{
    if (event != nullptr && event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }

    QWidget::changeEvent(event);
}
