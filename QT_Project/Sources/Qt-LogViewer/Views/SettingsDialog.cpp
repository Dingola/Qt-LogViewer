#include "Qt-LogViewer/Views/SettingsDialog.h"

#include <QDebug>

#include "Qt-LogViewer/Services/LogViewerSettings.h"
#include "ui_SettingsDialog.h"

/**
 * @brief Constructs a SettingsDialog and initializes the UI.
 * @param parent The parent widget, or nullptr.
 */
SettingsDialog::SettingsDialog(LogViewerSettings* settings, QWidget* parent)
    : QDialog(parent), ui(new Ui::SettingsDialog), m_settings(settings)
{
    ui->setupUi(this);

    connect(ui->pushButtonApply, &QPushButton::clicked, this, &SettingsDialog::onApply);
    connect(ui->pushButtonOk, &QPushButton::clicked, this, &SettingsDialog::onOk);
    connect(ui->pushButtonCancel, &QPushButton::clicked, this, &SettingsDialog::onCancel);

    if (m_settings != nullptr)
    {
        set_current_language(m_settings->get_language());
        set_current_theme(m_settings->get_theme());
    }
    else
    {
        set_current_language("en");
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
 * @param language_code The language code (e.g. "en", "de").
 */
auto SettingsDialog::set_current_language(const QString& language_code) -> void
{
    int index = ui->comboBoxLang->findData(language_code);

    if (ui->comboBoxLang->count() == 0)
    {
        ui->comboBoxLang->addItem(language_code);
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
 * @brief Returns the selected language code.
 * @return The selected language code.
 */
auto SettingsDialog::selected_language() const -> QString
{
    return ui->comboBoxLang->currentData().toString();
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
    ui->comboBoxLang->addItem(tr("English"), "en");
    ui->comboBoxLang->addItem(tr("German"), "de");
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
            m_settings->set_language(selected_language());
            emit language_changed(selected_language());
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
