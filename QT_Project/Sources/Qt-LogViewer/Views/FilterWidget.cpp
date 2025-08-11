#include "Qt-LogViewer/Views/FilterWidget.h"

#include <QSet>
#include <QString>

#include "ui_FilterWidget.h"

namespace
{
constexpr auto k_show_all_apps_text = QT_TRANSLATE_NOOP("FilterWidget", "Show All Apps");
constexpr auto k_show_all_apps_tooltip =
    QT_TRANSLATE_NOOP("FilterWidget", "Show logs from all applications");
}  // namespace

/**
 * @brief Constructs a FilterWidget object.
 *
 * Initializes the filter widget and its UI.
 *
 * @param parent The parent widget, or nullptr if this is a top-level widget.
 */
FilterWidget::FilterWidget(QWidget* parent): QWidget(parent), ui(new Ui::FilterWidget)
{
    ui->setupUi(this);
    ui->comboBoxApp->setFixedWidth(140);
    set_app_names({}, tr(k_show_all_apps_text), tr(k_show_all_apps_tooltip));

    if (ui->logLevelBar->layout() != nullptr)
    {
        ui->logLevelBar->layout()->setContentsMargins(6, 6, 6, 6);
    }

    // Connect app filter changes
    connect(ui->comboBoxApp, &QComboBox::currentTextChanged, this, [this](const QString& app_name) {
        emit app_filter_changed((app_name == tr(k_show_all_apps_text) ? QString() : app_name));
    });

    // Connect log level changes
    auto emit_log_levels = [this] { emit log_level_filter_changed(get_current_log_levels()); };
    connect(ui->checkBoxTrace, &QCheckBox::toggled, this, emit_log_levels);
    connect(ui->checkBoxDebug, &QCheckBox::toggled, this, emit_log_levels);
    connect(ui->checkBoxInfo, &QCheckBox::toggled, this, emit_log_levels);
    connect(ui->checkBoxWarning, &QCheckBox::toggled, this, emit_log_levels);
    connect(ui->checkBoxError, &QCheckBox::toggled, this, emit_log_levels);
    connect(ui->checkBoxFatal, &QCheckBox::toggled, this, emit_log_levels);
}

/**
 * @brief Destroys the FilterWidget object.
 *
 * Cleans up any resources used by the filter widget.
 */
FilterWidget::~FilterWidget()
{
    delete ui;
}

/**
 * @brief Populates the application filter combo box.
 *
 * Clears the combo box, adds a default entry, and populates it with the provided application names.
 * Disables the combo box if no application names are provided.
 *
 * @param app_names A set of application names to populate the combo box.
 * @param default_text The default entry text.
 * @param default_tooltip The tooltip for the default entry.
 */
auto FilterWidget::set_app_names(const QSet<QString>& app_names, const QString& default_text,
                                 const QString& default_tooltip) -> void
{
    ui->comboBoxApp->blockSignals(true);
    ui->comboBoxApp->clear();
    ui->comboBoxApp->addItem(default_text);
    ui->comboBoxApp->setItemData(0, default_tooltip, Qt::ToolTipRole);

    for (const QString& app: app_names)
    {
        if (!app.isEmpty())
        {
            ui->comboBoxApp->addItem(app);
        }
    }

    ui->comboBoxApp->setCurrentIndex(0);
    ui->comboBoxApp->setEnabled(!app_names.isEmpty());
    ui->comboBoxApp->blockSignals(false);
    qDebug() << "App combo box updated with" << app_names.size() << "apps.";
}

/**
 * @brief Sets the application names in the combo box.
 *
 * This overload sets the application names without a default entry.
 *
 * @param app_names The set of application names to populate the combo box.
 */
auto FilterWidget::set_app_names(const QSet<QString>& app_names) -> void
{
    set_app_names(app_names, tr(k_show_all_apps_text), tr(k_show_all_apps_tooltip));
}

/**
 * @brief Sets the checked state of log level checkboxes.
 *
 * @param levels The set of log levels to check.
 */
auto FilterWidget::set_log_levels(const QSet<QString>& levels) -> void
{
    ui->checkBoxTrace->setChecked(levels.contains("TRACE"));
    ui->checkBoxDebug->setChecked(levels.contains("DEBUG"));
    ui->checkBoxInfo->setChecked(levels.contains("INFO"));
    ui->checkBoxWarning->setChecked(levels.contains("WARNING"));
    ui->checkBoxError->setChecked(levels.contains("ERROR"));
    ui->checkBoxFatal->setChecked(levels.contains("FATAL"));
}

/**
 * @brief Returns the currently selected application name.
 *
 * @return The selected application name.
 */
auto FilterWidget::get_current_app_name() const -> QString
{
    return ui->comboBoxApp->currentText();
}

/**
 * @brief Returns the currently selected log levels.
 *
 * @return The set of selected log levels.
 */
auto FilterWidget::get_current_log_levels() const -> QSet<QString>
{
    QSet<QString> levels;

    if (ui->checkBoxTrace->isChecked())
    {
        levels.insert("TRACE");
    }
    if (ui->checkBoxDebug->isChecked())
    {
        levels.insert("DEBUG");
    }
    if (ui->checkBoxInfo->isChecked())
    {
        levels.insert("INFO");
    }
    if (ui->checkBoxWarning->isChecked())
    {
        levels.insert("WARNING");
    }
    if (ui->checkBoxError->isChecked())
    {
        levels.insert("ERROR");
    }
    if (ui->checkBoxFatal->isChecked())
    {
        levels.insert("FATAL");
    }

    return levels;
}

/**
 * @brief Handles change events to update the UI.
 * @param event The change event.
 */
auto FilterWidget::changeEvent(QEvent* event) -> void
{
    if (event != nullptr && event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }

    QWidget::changeEvent(event);
}
