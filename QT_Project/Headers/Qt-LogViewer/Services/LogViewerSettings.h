#pragma once

#include <QString>

#include "Qt-LogViewer/Services/AppPreferencesInterface.h"
#include "Qt-LogViewer/Services/Settings.h"

/**
 * @class LogViewerSettings
 * @brief Application-specific settings interface with type-safe getters/setters.
 *
 * Provides convenience methods for common application settings such as theme and language.
 * Inherits from Settings and can be extended for further application-specific settings.
 */
class LogViewerSettings: public Settings, public AppPreferencesInterface
{
        Q_OBJECT
        Q_INTERFACES(AppPreferencesInterface)

    public:
        using Settings::Settings;

        ~LogViewerSettings() override = default;

        /**
         * @brief Returns the current theme.
         * @return The theme name (e.g. "Dark", "Light"). Default is "Dark".
         */
        [[nodiscard]] auto get_theme() -> QString override;

        /**
         * @brief Sets the theme.
         * @param value The theme name to set (e.g. "Dark", "Light").
         */
        auto set_theme(const QString& value) -> void override;

        /**
         * @brief Returns the current language code.
         * @return The language code (e.g. "en", "de"). Default is "en".
         */
        [[nodiscard]] auto get_language() -> QString override;

        /**
         * @brief Sets the language code.
         * @param value The language code to set (e.g. "en", "de").
         */
        auto set_language(const QString& value) -> void override;

    signals:
        /**
         * @brief Emitted when the language is changed.
         * @param language_code The new language code.
         */
        void languageChanged(const QString& language_code) override;

        /**
         * @brief Emitted when the theme is changed.
         * @param theme_name The new theme name.
         */
        void themeChanged(const QString& theme_name) override;
};
