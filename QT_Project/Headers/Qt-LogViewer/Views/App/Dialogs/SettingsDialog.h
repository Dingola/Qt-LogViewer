#pragma once

#include <QColor>
#include <QString>
#include <QStringList>

#include "Qt-LogViewer/Views/Shared/Dialog.h"

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
class SettingsDialog: public Dialog
{
        Q_OBJECT
        Q_PROPERTY(QColor storage_location_icon_color READ get_storage_location_icon_color WRITE
                       set_storage_location_icon_color)
        Q_PROPERTY(
            QColor storage_location_icon_color_hover READ get_storage_location_icon_color_hover
                WRITE set_storage_location_icon_color_hover)
        Q_PROPERTY(int storage_location_icon_px READ get_storage_location_icon_px WRITE
                       set_storage_location_icon_px)

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
         * @param language_name The language name (e.g. "English").
         */
        auto set_current_language(const QString& language_name) -> void;

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
         * @brief Sets the language code to name mapping.
         * @param language_code_name_map The map of language codes to names.
         */
        auto set_available_language_names(const QStringList& language_names) -> void;

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

        /**
         * @brief Returns the normal color of the storage location icons.
         * @return The configured normal icon color.
         */
        [[nodiscard]] auto get_storage_location_icon_color() const -> QColor;

        /**
         * @brief Sets the normal color of the storage location icons.
         * @param color The new normal icon color.
         */
        auto set_storage_location_icon_color(const QColor& color) -> void;

        /**
         * @brief Returns the hover color of the storage location icons.
         * @return The configured hover icon color.
         */
        [[nodiscard]] auto get_storage_location_icon_color_hover() const -> QColor;

        /**
         * @brief Sets the hover color of the storage location icons.
         * @param color The new hover icon color.
         */
        auto set_storage_location_icon_color_hover(const QColor& color) -> void;

        /**
         * @brief Returns the square storage location icon size.
         * @return The icon size in pixels.
         */
        [[nodiscard]] auto get_storage_location_icon_px() const -> int;

        /**
         * @brief Sets the square storage location icon size.
         * @param size The icon size in pixels.
         */
        auto set_storage_location_icon_px(int size) -> void;

    private:
        /**
         * @brief Initializes the category list with items.
         */
        auto setup_categories() -> void;

        /**
         * @brief Displays canonical application storage locations and connects their open buttons.
         */
        auto setup_storage_locations() -> void;

        /**
         * @brief Applies the configured icon appearance to every storage location button.
         */
        auto update_storage_location_icons() -> void;

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

    protected:
        /**
         * @brief Handles change events to update the UI.
         * @param event The change event.
         */
        auto changeEvent(QEvent* event) -> void override;

    signals:
        /**
         * @brief Emitted when the language is changed.
         * @param language_code The new language name.
         */
        auto language_name_changed(const QString& language_name) -> void;

        /**
         * @brief Emitted when the theme is changed.
         * @param theme_name The new theme name.
         */
        auto theme_changed(const QString& theme_name) -> void;

    private slots:
        auto onApply() -> void;
        auto onOk() -> void;
        auto onCancel() -> void;
        auto onCategoryChanged(int index) -> void;

    private:
        Ui::SettingsDialog* ui =
            nullptr;  ///< Pointer to the UI definition generated from the .ui file.

        LogViewerSettings* m_settings =
            nullptr;                 ///< Settings object for managing application settings.
        QString m_applied_language;  ///< The language that was applied when the dialog was opened.
        QString m_applied_theme;     ///< The theme that was applied when the dialog was opened.
        QStringList m_available_themes;          ///< List of available theme names.
        QStringList m_available_language_names;  ///< List of available language names.
        QColor m_storage_location_icon_color =
            QColor("#66bb6a");  ///< Normal storage location icon color.
        QColor m_storage_location_icon_color_hover =
            QColor("#ffffff");                ///< Hovered storage location icon color.
        int m_storage_location_icon_px = 17;  ///< Square storage location icon size in pixels.
};
