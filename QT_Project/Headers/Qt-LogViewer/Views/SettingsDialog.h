#pragma once

#include <QDialog>
#include <QString>
#include <QStringList>

namespace Ui
{
class SettingsDialog;
}

class LogViewerSettings;

/**
 * @class SettingsDialog
 * @brief Dialog for changing application settings such as language and theme.
 *
 * Provides UI for selecting language and theme. Emits signals when settings are changed.
 * The dialog supports Apply, OK, and Cancel logic.
 */
class SettingsDialog: public QDialog
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs a SettingsDialog.
         * @param parent The parent widget, or nullptr.
         */
        explicit SettingsDialog(LogViewerSettings* settings, QWidget* parent = nullptr);

        /**
         * @brief Destroys the SettingsDialog.
         */
        ~SettingsDialog() override;

        /**
         * @brief Sets the current language selection.
         * @param language_code The language code (e.g. "en", "de").
         */
        auto set_current_language(const QString& language_code) -> void;

        /**
         * @brief Sets the current theme selection.
         * @param theme_name The theme name (e.g. "Dark").
         */
        auto set_current_theme(const QString& theme_name) -> void;

        /**
         * @brief Sets the list of available themes and updates the theme combo box.
         *
         * This method assigns the provided list of theme names to the dialog and refreshes
         * the theme selection UI accordingly.
         *
         * @param themes The list of available theme names.
         */
        auto set_available_themes(const QStringList& themes) -> void;

        /**
         * @brief Returns the selected language code.
         * @return The selected language code.
         */
        [[nodiscard]] auto selected_language() const -> QString;

        /**
         * @brief Returns the selected theme name.
         * @return The selected theme name.
         */
        [[nodiscard]] auto selected_theme() const -> QString;

    private:
        /**
         * @brief Populates the language combo box.
         */
        auto load_available_languages() -> void;

        /**
         * @brief Populates the theme combo box.
         */
        auto load_available_themes() -> void;

        /**
         * @brief Applies the changes and emits signals if needed.
         */
        auto apply_changes() -> void;

        /**
         * @brief Reverts the selections to the initial values.
         */
        auto revert_changes() -> void;

    signals:
        /**
         * @brief Emitted when the language is changed.
         * @param language_code The new language code.
         */
        auto language_changed(const QString& language_code) -> void;

        /**
         * @brief Emitted when the theme is changed.
         * @param theme_name The new theme name.
         */
        auto theme_changed(const QString& theme_name) -> void;

    private slots:
        auto onApply() -> void;
        auto onOk() -> void;
        auto onCancel() -> void;

    private:
        Ui::SettingsDialog* ui = nullptr;

        LogViewerSettings* m_settings = nullptr;
        QString m_applied_language;
        QString m_applied_theme;
        QStringList m_available_themes;
};
