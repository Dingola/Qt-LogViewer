#include "Qt-LogViewer/Views/App/LogFilterWidget.h"

#include <QHBoxLayout>
#include <QMap>
#include <QSet>
#include <QString>
#include <QVector>

#include "Qt-LogViewer/Views/App/LogLevelFilterItemWidget.h"
#include "ui_LogFilterWidget.h"

namespace
{
constexpr auto k_show_all_apps_text = QT_TRANSLATE_NOOP("LogFilterWidget", "Show All Apps");
constexpr auto k_show_all_apps_tooltip =
    QT_TRANSLATE_NOOP("LogFilterWidget", "Show logs from all applications");
}  // namespace

/**
 * @brief Constructs a LogFilterWidget object.
 *
 * Initializes the filter widget and its UI.
 *
 * @param parent The parent widget, or nullptr if this is a top-level widget.
 */
LogFilterWidget::LogFilterWidget(QWidget* parent): QWidget(parent), ui(new Ui::LogFilterWidget)
{
    ui->setupUi(this);
    ui->comboBoxApp->setFixedWidth(140);
    set_app_names({}, tr(k_show_all_apps_text), tr(k_show_all_apps_tooltip));

    QLayout* layout = ui->logLevelBar->layout();

    if (layout == nullptr)
    {
        layout = new QHBoxLayout(ui->logLevelBar);
        ui->logLevelBar->setLayout(layout);
    }

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    ui->horizontalLayout->setContentsMargins(0, 0, 0, 0);

    // Connect app filter changes
    connect(ui->comboBoxApp, &QComboBox::currentTextChanged, this, [this](const QString& app_name) {
        emit app_filter_changed((app_name == tr(k_show_all_apps_text) ? QString() : app_name));
    });
}

/**
 * @brief Destroys the LogFilterWidget object.
 *
 * Cleans up any resources used by the filter widget.
 */
LogFilterWidget::~LogFilterWidget()
{
    delete ui;
}

/**
 * @brief Sets the currently selected application name in the combo box.
 *
 * If the provided application name does not exist in the combo box, no selection is made.
 *
 * @param app_name The application name to select.
 */
auto LogFilterWidget::set_current_app_name_filter(const QString& app_name) -> void
{
    int index = ui->comboBoxApp->findText(app_name);

    if (index != -1)
    {
        ui->comboBoxApp->setCurrentIndex(index);
    }
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
auto LogFilterWidget::set_app_names(const QSet<QString>& app_names, const QString& default_text,
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
auto LogFilterWidget::set_app_names(const QSet<QString>& app_names) -> void
{
    set_app_names(app_names, tr(k_show_all_apps_text), tr(k_show_all_apps_tooltip));
}

/**
 * @brief Sets the available log levels and creates filter items dynamically.
 *        Log level names are normalized for matching.
 *
 * Removes any existing log level filter items and creates new ones for each log level.
 * Each item's checkbox receives a unique objectName for QSS styling.
 *
 * @param log_levels List of log level names (e.g., "TRACE", "DEBUG").
 */
auto LogFilterWidget::set_available_log_levels(const QVector<QString>& log_levels) -> void
{
    // Remove old items
    QLayout* layout = ui->logLevelBar->layout();

    if (layout != nullptr)
    {
        QLayoutItem* item;
        while ((item = layout->takeAt(0)) != nullptr)
        {
            QWidget* widget = item->widget();

            if (widget != nullptr)
            {
                widget->deleteLater();
            }

            delete item;
        }
    }

    m_log_level_items.clear();

    // Add new items
    for (const QString& level: log_levels)
    {
        auto* item_widget = new LogLevelFilterItemWidget(ui->logLevelBar);
        item_widget->set_log_level_name(level);
        item_widget->set_count_text("0");
        item_widget->set_checked(false);

        connect(item_widget, &LogLevelFilterItemWidget::toggled, this,
                [this] { emit log_level_filter_changed(get_current_log_levels()); });

        layout->addWidget(item_widget);
        const QString normalized_level = level.trimmed().toLower();
        m_log_level_items[normalized_level] = item_widget;
    }
}

/**
 * @brief Sets the checked state of log level filter items.
 *        Log level names are normalized for matching.
 *
 * @param levels The set of log levels to check.
 */
auto LogFilterWidget::set_log_levels(const QSet<QString>& levels) -> void
{
    QSet<QString> normalized_levels;

    for (const auto& level: levels)
    {
        normalized_levels.insert(level.trimmed().toLower());
    }

    for (auto it = m_log_level_items.begin(); it != m_log_level_items.end(); ++it)
    {
        it.value()->set_checked(normalized_levels.contains(it.key()));
    }
}

/**
 * @brief Sets the count for each log level and updates the UI.
 *        Log level names are normalized for matching.
 *
 * @param level_counts Map of log level name to count.
 */
auto LogFilterWidget::set_log_level_counts(const QMap<QString, int>& level_counts) -> void
{
    QMap<QString, int> normalized_counts;

    for (auto it = level_counts.begin(); it != level_counts.end(); ++it)
    {
        normalized_counts[it.key().trimmed().toLower()] = it.value();
    }

    for (auto it = m_log_level_items.begin(); it != m_log_level_items.end(); ++it)
    {
        int count = normalized_counts.value(it.key(), 0);
        it.value()->set_count(count);
    }
}

/**
 * @brief Returns the currently selected application name.
 *
 * @return The selected application name.
 */
auto LogFilterWidget::get_current_app_name() const -> QString
{
    return ui->comboBoxApp->currentText();
}

/**
 * @brief Returns the currently selected log levels.
 *
 * @return The set of selected log levels.
 */
auto LogFilterWidget::get_current_log_levels() const -> QSet<QString>
{
    QSet<QString> levels;

    for (auto it = m_log_level_items.begin(); it != m_log_level_items.end(); ++it)
    {
        if (it.value()->get_checked())
        {
            levels.insert(it.key());
        }
    }

    return levels;
}

/**
 * @brief Handles change events to update the UI.
 * @param event The change event.
 */
auto LogFilterWidget::changeEvent(QEvent* event) -> void
{
    if (event != nullptr && event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }

    QWidget::changeEvent(event);
}
