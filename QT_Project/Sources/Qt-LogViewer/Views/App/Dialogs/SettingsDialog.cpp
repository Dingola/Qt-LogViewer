#include "Qt-LogViewer/Views/App/Dialogs/SettingsDialog.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QLineEdit>
#include <QToolButton>

#include "Qt-LogViewer/Services/ApplicationStoragePaths.h"
#include "Qt-LogViewer/Services/DesktopServices.h"
#include "Qt-LogViewer/Services/LogViewerSettings.h"
#include "QtWidgetsCommonLib/Utils/UiUtils.h"
#include "ui_SettingsDialog.h"

namespace
{
/**
 * @brief Presents a storage path with its distinguishing file or directory name visible.
 * @param line_edit Read-only field displaying the path.
 * @param path Absolute file or directory path to display.
 */
auto present_storage_path(QLineEdit* line_edit, const QString& path) -> void
{
    if (line_edit != nullptr)
    {
        line_edit->setText(path);
        line_edit->setToolTip(path);
        line_edit->setCursorPosition(path.size());
    }
}
}  // namespace

/**
 * @brief Constructs a SettingsDialog and initializes the UI.
 * @param settings The settings object for managing application preferences.
 * @param parent The parent widget, or nullptr.
 */
SettingsDialog::SettingsDialog(LogViewerSettings* settings, QWidget* parent)
    : Dialog(tr("Settings"), parent, Dialog::LayoutMode::External),
      ui(new Ui::SettingsDialog),
      m_settings(settings)
{
    ui->setupUi(this);

    setup_categories();

    connect(ui->listWidgetCategories, &QListWidget::currentRowChanged, this,
            &SettingsDialog::onCategoryChanged);
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

    ui->listWidgetCategories->setProperty("isContent", true);
    setup_storage_locations();

    // Set initial category
    ui->listWidgetCategories->setCurrentRow(0);
    ui->listWidgetCategories->setFocusPolicy(Qt::NoFocus);
}

/**
 * @brief Destroys the SettingsDialog.
 */
SettingsDialog::~SettingsDialog()
{
    delete ui;
}

/**
 * @brief Initializes the category list with items.
 */
auto SettingsDialog::setup_categories() -> void
{
    ui->listWidgetCategories->clear();
    ui->listWidgetCategories->addItem(tr("General"));
    ui->listWidgetCategories->addItem(tr("Storage"));
    ui->listWidgetCategories->addItem(tr("Appearance"));
}

/**
 * @brief Displays canonical application storage locations and connects their open buttons.
 */
auto SettingsDialog::setup_storage_locations() -> void
{
    const ApplicationStoragePaths storage_paths;
    const QString settings_file =
        m_settings != nullptr ? m_settings->fileName() : tr("No settings file");
    const QString settings_directory = m_settings != nullptr
                                           ? QFileInfo(settings_file).absolutePath()
                                           : storage_paths.get_application_root();
    const QString sessions_directory = storage_paths.get_sessions_directory();
    const QString cache_directory = storage_paths.get_cache_directory();
    const QString diagnostics_directory = storage_paths.get_diagnostics_directory();

    present_storage_path(ui->lineEditSettingsLocation, settings_file);
    present_storage_path(ui->lineEditSessionsLocation, sessions_directory);
    present_storage_path(ui->lineEditCacheLocation, cache_directory);
    present_storage_path(ui->lineEditDiagnosticsLocation, diagnostics_directory);

    update_storage_location_icons();

    /** @brief Opens the folder containing the application settings file. */
    connect(ui->toolButtonSettingsLocation, &QToolButton::clicked, this, [settings_directory]() {
        QDir().mkpath(settings_directory);
        const bool folder_opened = DesktopServices::open_folder(settings_directory);
        Q_UNUSED(folder_opened);
    });
    /** @brief Opens the directory containing saved sessions. */
    connect(ui->toolButtonSessionsLocation, &QToolButton::clicked, this, [sessions_directory]() {
        QDir().mkpath(sessions_directory);
        const bool folder_opened = DesktopServices::open_folder(sessions_directory);
        Q_UNUSED(folder_opened);
    });
    /** @brief Opens the directory containing reconstructible cache data. */
    connect(ui->toolButtonCacheLocation, &QToolButton::clicked, this, [cache_directory]() {
        QDir().mkpath(cache_directory);
        const bool folder_opened = DesktopServices::open_folder(cache_directory);
        Q_UNUSED(folder_opened);
    });
    /** @brief Opens the directory containing diagnostic logs. */
    connect(ui->toolButtonDiagnosticsLocation, &QToolButton::clicked, this,
            [diagnostics_directory]() {
                QDir().mkpath(diagnostics_directory);
                const bool folder_opened = DesktopServices::open_folder(diagnostics_directory);
                Q_UNUSED(folder_opened);
            });
}

/**
 * @brief Returns the normal color of the storage location icons.
 * @return The configured normal icon color.
 */
auto SettingsDialog::get_storage_location_icon_color() const -> QColor
{
    return m_storage_location_icon_color;
}

/**
 * @brief Sets the normal color of the storage location icons.
 * @param color The new normal icon color.
 */
auto SettingsDialog::set_storage_location_icon_color(const QColor& color) -> void
{
    if (m_storage_location_icon_color != color)
    {
        m_storage_location_icon_color = color;
        update_storage_location_icons();
    }
}

/**
 * @brief Returns the hover color of the storage location icons.
 * @return The configured hover icon color.
 */
auto SettingsDialog::get_storage_location_icon_color_hover() const -> QColor
{
    return m_storage_location_icon_color_hover;
}

/**
 * @brief Sets the hover color of the storage location icons.
 * @param color The new hover icon color.
 */
auto SettingsDialog::set_storage_location_icon_color_hover(const QColor& color) -> void
{
    if (m_storage_location_icon_color_hover != color)
    {
        m_storage_location_icon_color_hover = color;
        update_storage_location_icons();
    }
}

/**
 * @brief Returns the square storage location icon size.
 * @return The icon size in pixels.
 */
auto SettingsDialog::get_storage_location_icon_px() const -> int
{
    return m_storage_location_icon_px;
}

/**
 * @brief Sets the square storage location icon size.
 * @param size The icon size in pixels.
 */
auto SettingsDialog::set_storage_location_icon_px(int size) -> void
{
    const int bounded_size = qMax(1, size);
    if (m_storage_location_icon_px != bounded_size)
    {
        m_storage_location_icon_px = bounded_size;
        update_storage_location_icons();
    }
}

/** @brief Applies the configured icon appearance to every storage location button. */
auto SettingsDialog::update_storage_location_icons() -> void
{
    if (ui != nullptr)
    {
        const QSize icon_size(m_storage_location_icon_px, m_storage_location_icon_px);
        QIcon icon = QtWidgetsCommonLib::UiUtils::colored_svg_icon(
            QStringLiteral(":/Resources/Icons/folder-open-external.svg"),
            m_storage_location_icon_color, icon_size);
        icon.addPixmap(QtWidgetsCommonLib::UiUtils::colored_svg_icon(
                           QStringLiteral(":/Resources/Icons/folder-open-external.svg"),
                           m_storage_location_icon_color_hover, icon_size),
                       QIcon::Active);
        const QList<QToolButton*> location_buttons{
            ui->toolButtonSettingsLocation, ui->toolButtonSessionsLocation,
            ui->toolButtonCacheLocation, ui->toolButtonDiagnosticsLocation};
        for (QToolButton* button: location_buttons)
        {
            button->setIcon(icon);
            button->setIconSize(icon_size);
        }
    }
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
 * @brief Slot: Handles category selection change.
 * @param index The index of the selected category.
 */
auto SettingsDialog::onCategoryChanged(int index) -> void
{
    ui->stackedWidgetContent->setCurrentIndex(index);
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
        setup_categories();
        load_available_languages();
        load_available_themes();
    }

    QDialog::changeEvent(event);
}
