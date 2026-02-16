#pragma once

#include <QString>
#include <QtPlugin>

/**
 * @class IThemePreferences
 * @brief Interface for application theme preferences.
 *
 * This interface defines type-safe getters and setters for the application theme.
 */
class IThemePreferences
{
    public:
        /**
         * @brief Virtual destructor for safe polymorphic use.
         */
        virtual ~IThemePreferences() = default;

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

    signals:
        /**
         * @brief Emitted when the theme is changed.
         * @param theme_name The new theme name.
         */
        virtual void themeChanged(const QString& theme_name) = 0;
};

Q_DECLARE_INTERFACE(IThemePreferences, "de.adrianhelbig.IThemePreferences")
