#pragma once

#include <QString>
#include <QtPlugin>

/**
 * @class ILanguagePreferences
 * @brief Interface for application language preferences.
 *
 * This interface defines type-safe getters and setters for the application language settings.
 */
class ILanguagePreferences
{
    public:
        /**
         * @brief Virtual destructor for safe polymorphic use.
         */
        virtual ~ILanguagePreferences() = default;

        /**
         * @brief Returns the current language code.
         * @return The language code (e.g. "en", "de").
         */
        [[nodiscard]] virtual auto get_language_code() -> QString = 0;

        /**
         * @brief Sets the language code.
         * @param value The language code to set (e.g. "en", "de").
         */
        virtual auto set_language_code(const QString& language_code) -> void = 0;

        /**
         * @brief Returns the current language name (e.g. "English", "German").
         */
        [[nodiscard]] virtual auto get_language_name() -> QString = 0;

        /**
         * @brief Sets the language name.
         * @param language_name The language name to set (e.g. "English", "German").
         */
        virtual auto set_language_name(const QString& language_name) -> void = 0;

    signals:
        /**
         * @brief Emitted when the language is changed.
         * @param language_code The new language code.
         */
        virtual void languageCodeChanged(const QString& language_code) = 0;

        /**
         * @brief Emitted when the language name is changed.
         * @param language_name The new language name.
         */
        virtual void languageNameChanged(const QString& language_name) = 0;
};

Q_DECLARE_INTERFACE(ILanguagePreferences, "de.adrianhelbig.ILanguagePreferences")
