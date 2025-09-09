#include "Qt-LogViewer/Views/App/Dialogs/SettingsDialog.h"

#include <QDebug>

#include "Qt-LogViewer/Services/LogViewerSettings.h"
#include "ui_SettingsDialog.h"

/**
 * @brief Constructs a SettingsDialog and initializes the UI.
 * @param parent The parent widget, or nullptr.
 */
SettingsDialog::SettingsDialog(LogViewerSettings* settings, QWidget* parent)
    : Dialog(tr("Settings"), parent), ui(new Ui::SettingsDialog), m_settings(settings)
{
    ui->setupUi(this);

    auto* vlayout = qobject_cast<QVBoxLayout*>(layout());
    if (vlayout)
    {
        vlayout->insertWidget(0, header_widget());
    }

    connect(ui->pushButtonApply, &QPushButton::clicked, this, &SettingsDialog::onApply);
    connect(ui->pushButtonOk, &QPushButton::clicked, this, &SettingsDialog::onOk);
    connect(ui->pushButtonCancel, &QPushButton::clicked, this, &SettingsDialog::onCancel);

    if (m_settings != nullptr)
    {
        set_current_language(m_settings->get_language_name());
        set_current_theme(m_settings->get_theme());
        connect(m_settings, &LogViewerSettings::languageCodeChanged, this,
                &SettingsDialog::set_current_language);
        connect(m_settings, &LogViewerSettings::themeChanged, this,
                &SettingsDialog::set_current_theme);
    }
    else
    {
        set_current_language("English");
        set_current_theme("Dark");
        qWarning() << "No LogViewerSettings provided, using defaults.";
    }

    m_applied_language = selected_language();
    m_applied_theme = selected_theme();
}

/**
 * @brief Destroys the SettingsDialog.
 */
SettingsDialog::~SettingsDialog()
{
    delete ui;
}

/**
 * @brief Sets the current language selection.
 * @param language_name The language name (e.g. "English").
 */
auto SettingsDialog::set_current_language(const QString& language_name) -> void
{
    int index = ui->comboBoxLang->findText(language_name);

    if (ui->comboBoxLang->count() == 0)
    {
        ui->comboBoxLang->addItem(language_name);
    }

    if (index >= 0)
    {
        ui->comboBoxLang->setCurrentIndex(index);
    }
    else if (ui->comboBoxLang->count() > 0)
    {
        ui->comboBoxLang->setCurrentIndex(0);
    }

    m_applied_language = selected_language();
}

/**
 * @brief Sets the current theme selection.
 * @param theme_name The theme name (e.g. "Dark").
 */
auto SettingsDialog::set_current_theme(const QString& theme_name) -> void
{
    int index = ui->comboBoxTheme->findText(theme_name);

    if (ui->comboBoxTheme->count() == 0)
    {
        ui->comboBoxTheme->addItem(theme_name);
    }

    if (index >= 0)
    {
        ui->comboBoxTheme->setCurrentIndex(index);
    }
    else if (ui->comboBoxTheme->count() > 0)
    {
        ui->comboBoxTheme->setCurrentIndex(0);
    }

    m_applied_theme = selected_theme();
}

/**
 * @brief Sets the list of available themes and updates the theme combo box.
 *
 * This method assigns the provided list of theme names to the dialog and refreshes
 * the theme selection UI accordingly.
 *
 * @param themes The list of available theme names.
 */
auto SettingsDialog::set_available_themes(const QStringList& themes) -> void
{
    m_available_themes = themes;
    load_available_themes();
}

/**
 * @brief Sets the language code to name mapping.
 * @param language_code_name_map The map of language codes to names.
 */
auto SettingsDialog::set_available_language_names(const QStringList& language_names) -> void
{
    m_available_language_names = language_names;
    load_available_languages();
}

/**
 * @brief Returns the selected language code.
 * @return The selected language code.
 */
auto SettingsDialog::selected_language() const -> QString
{
    return ui->comboBoxLang->currentText();
}

/**
 * @brief Returns the selected theme name.
 * @return The selected theme name.
 */
auto SettingsDialog::selected_theme() const -> QString
{
    return ui->comboBoxTheme->currentText();
}

/**
 * @brief Populates the language combo box.
 */
auto SettingsDialog::load_available_languages() -> void
{
    ui->comboBoxLang->clear();

    for (auto& language_name: m_available_language_names)
    {
        ui->comboBoxLang->addItem(language_name);
    }

    set_current_language(m_applied_language);
}

/**
 * @brief Populates the theme combo box.
 */
auto SettingsDialog::load_available_themes() -> void
{
    ui->comboBoxTheme->clear();
    ui->comboBoxTheme->addItems(m_available_themes);
    set_current_theme(m_applied_theme);
}

/**
 * @brief Slot: Handles Apply button click.
 */
auto SettingsDialog::onApply() -> void
{
    apply_changes();
    m_applied_language = selected_language();
    m_applied_theme = selected_theme();
}

/**
 * @brief Slot: Handles OK button click.
 */
auto SettingsDialog::onOk() -> void
{
    apply_changes();
    accept();
}

/**
 * @brief Slot: Handles Cancel button click.
 */
auto SettingsDialog::onCancel() -> void
{
    revert_changes();
    reject();
}

/**
 * @brief Applies the changes and emits signals if needed.
 */
auto SettingsDialog::apply_changes() -> void
{
    if (selected_language() != m_applied_language)
    {
        if (m_settings != nullptr)
        {
            m_settings->set_language_name(selected_language());
            emit language_name_changed(selected_language());
        }
        else
        {
            qWarning() << "No LogViewerSettings provided, cannot apply language change.";
        }
    }

    if (selected_theme() != m_applied_theme)
    {
        if (m_settings != nullptr)
        {
            m_settings->set_theme(selected_theme());
            emit theme_changed(selected_theme());
        }
        else
        {
            qWarning() << "No LogViewerSettings provided, cannot apply theme change.";
        }
    }
}

/**
 * @brief Reverts the selections to the initial values.
 */
auto SettingsDialog::revert_changes() -> void
{
    set_current_language(m_applied_language);
    set_current_theme(m_applied_theme);
}

/**
 * @brief Handles change events to update the UI.
 * @param event The change event.
 */
auto SettingsDialog::changeEvent(QEvent* event) -> void
{
    if (event != nullptr && event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        load_available_languages();
        load_available_themes();
    }

    QDialog::changeEvent(event);
}
