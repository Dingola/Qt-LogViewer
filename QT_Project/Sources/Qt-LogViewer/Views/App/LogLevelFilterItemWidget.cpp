#include "Qt-LogViewer/Views/App/LogLevelFilterItemWidget.h"

#include "Qt-LogViewer/Services/NumberFormatUtils.h"
#include "ui_LogLevelFilterItemWidget.h"

/**
 * @brief Constructs a LogLevelFilterItemWidget object.
 *
 * Initializes the log level filter item widget and its UI.
 *
 * @param parent The parent widget, or nullptr if this is a top-level widget.
 */
LogLevelFilterItemWidget::LogLevelFilterItemWidget(QWidget* parent)
    : QWidget(parent), ui(new Ui::LogLevelFilterItemWidget), m_log_level_name()
{
    ui->setupUi(this);
    ui->horizontalLayout->setContentsMargins(6, 6, 6, 6);
    ui->horizontalLayout->setSpacing(2);
    connect(ui->checkBox, &QCheckBox::toggled, this, &LogLevelFilterItemWidget::toggled);
}

/**
 * @brief Destroys the LogLevelFilterItemWidget object.
 *
 * Cleans up any resources used by the widget.
 */
LogLevelFilterItemWidget::~LogLevelFilterItemWidget()
{
    delete ui;
}

/**
 * @brief Sets the log level name and checkbox text.
 *        Also sets the checkbox objectName for QSS styling.
 * @param level_name The log level name (e.g., "Trace", "Debug").
 */
auto LogLevelFilterItemWidget::set_log_level_name(const QString& level_name) -> void
{
    m_log_level_name = level_name;
    ui->checkBox->setText(level_name);
    ui->checkBox->setObjectName("checkBox" + level_name);
}

/**
 * @brief Sets the count label for the log level.
 * @param count_text The text to display (e.g., "42", "1.2K").
 */
auto LogLevelFilterItemWidget::set_count_text(const QString& count_text) -> void
{
    ui->labelCount->setText(count_text);
}

/**
 * @brief Sets the count label for the log level.
 * @param count The integer count to display (e.g., 42, 1200).
 */
auto LogLevelFilterItemWidget::set_count(int count) -> void
{
    ui->labelCount->setText(NumberFormatUtils::format_number_abbreviated(count));
}

/**
 * @brief Sets the checked state of the checkbox.
 * @param checked True to check, false to uncheck.
 */
auto LogLevelFilterItemWidget::set_checked(bool checked) -> void
{
    ui->checkBox->setChecked(checked);
}

/**
 * @brief Returns whether the checkbox is checked.
 * @return True if checked, false otherwise.
 */
auto LogLevelFilterItemWidget::get_checked() const -> bool
{
    return ui->checkBox->isChecked();
}

/**
 * @brief Returns the log level name.
 * @return The log level name.
 */
auto LogLevelFilterItemWidget::get_log_level_name() const -> QString
{
    return m_log_level_name;
}

/**
 * @brief Resets the count label to "0" and unchecks the checkbox.
 */
auto LogLevelFilterItemWidget::clear() -> void
{
    ui->labelCount->setText("0");
    ui->checkBox->setChecked(false);
}

/**
 * @brief Handles change events to update the UI.
 * @param event The change event.
 */
auto LogLevelFilterItemWidget::changeEvent(QEvent* event) -> void
{
    if (event != nullptr && event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }

    QWidget::changeEvent(event);
}
