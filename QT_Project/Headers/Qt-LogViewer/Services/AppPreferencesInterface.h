#pragma once

#include <QString>
#include <QtPlugin>

/**
 * @class AppPreferencesInterface
 * @brief Interface for application-wide user preferences such as theme and language.
 *
 * This interface defines type-safe getters and setters for common user preferences
 * like application theme and language. Classes implementing this interface should
 * provide persistent storage and retrieval of these preferences.
 *
 */
class AppPreferencesInterface
{
    public:
        /**
         * @brief Virtual destructor for safe polymorphic use.
         */
        virtual ~AppPreferencesInterface() = default;

        /**
         * @brief Returns the current application theme.
         * @return The theme name (e.g. "Dark", "Light").
         */
        [[nodiscard]] virtual auto get_theme() -> QString = 0;

        /**
         * @brief Sets the application theme.
         * @param value The theme name to set (e.g. "Dark", "Light").
         */
        virtual auto set_theme(const QString& value) -> void = 0;

        /**
         * @brief Returns the current language code.
         * @return The language code (e.g. "en", "de").
         */
        [[nodiscard]] virtual auto get_language() -> QString = 0;

        /**
         * @brief Sets the language code.
         * @param value The language code to set (e.g. "en", "de").
         */
        virtual auto set_language(const QString& value) -> void = 0;

    signals:
        /**
         * @brief Emitted when the language is changed.
         * @param language_code The new language code.
         */
        virtual void languageChanged(const QString& language_code) = 0;

        /**
         * @brief Emitted when the theme is changed.
         * @param theme_name The new theme name.
         */
        virtual void themeChanged(const QString& theme_name) = 0;
};

Q_DECLARE_INTERFACE(AppPreferencesInterface, "de.adrianhelbig.AppPreferencesInterface")