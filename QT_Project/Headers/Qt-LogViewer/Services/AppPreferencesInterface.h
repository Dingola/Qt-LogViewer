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

        /**
         * @brief Returns the last saved main window geometry (position and size).
         * @param geometry The geometry data to save.
         */
        [[nodiscard]] virtual auto get_mainwindow_geometry() -> QByteArray = 0;

        /**
         * @brief Sets the main window geometry (position and size).
         * @param geometry The geometry data to save.
         */
        virtual auto set_mainwindow_geometry(const QByteArray& geometry) -> void = 0;

        /**
         * @brief Returns the last saved main window state.
         * @return The state data as a QByteArray.
         */
        [[nodiscard]] virtual auto get_mainwindow_state() -> QByteArray = 0;

        /**
         * @brief Sets the main window state.
         * @param state The state data to save as a QByteArray.
         */
        virtual auto set_mainwindow_state(const QByteArray& state) -> void = 0;

        /**
         * @brief Returns the last saved main window window state.
         * @return The window state as an integer (e.g. Qt::WindowNoState, Qt::WindowMinimized,
         * etc.).
         */
        [[nodiscard]] virtual auto get_mainwindow_windowstate() -> int = 0;

        /**
         * @brief Sets the main window window state.
         * @param state The window state to save as an integer (e.g. Qt::WindowNoState,
         * Qt::WindowMinimized, etc.).
         */
        virtual auto set_mainwindow_windowstate(int state) -> void = 0;

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