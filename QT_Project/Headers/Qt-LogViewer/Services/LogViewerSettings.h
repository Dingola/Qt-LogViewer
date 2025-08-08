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
        [[nodiscard]] auto get_language_code() -> QString override;

        /**
         * @brief Sets the language code.
         * @param value The language code to set (e.g. "en", "de").
         */
        auto set_language_code(const QString& language_code) -> void override;

        /**
         * @brief Returns the current language name (e.g. "English", "Deutsch").
         */
        [[nodiscard]] auto get_language_name() -> QString override;

        /**
         * @brief Sets the language name.
         * @param language_name The language name to set (e.g. "English", "Deutsch").
         */
        auto set_language_name(const QString& language_name) -> void override;

        /**
         * @brief Returns the last saved main window geometry (position and size).
         * @return The geometry data as QByteArray.
         */
        [[nodiscard]] auto get_mainwindow_geometry() -> QByteArray override;

        /**
         * @brief Sets the main window geometry (position and size).
         * @param geometry The geometry data to save as QByteArray.
         */
        auto set_mainwindow_geometry(const QByteArray& geometry) -> void override;

        /**
         * @brief Returns the last saved main window state.
         * @return The state data as a QByteArray.
         */
        [[nodiscard]] auto get_mainwindow_state() -> QByteArray override;

        /**
         * @brief Sets the main window state.
         * @param state The state data to save as a QByteArray.
         */
        auto set_mainwindow_state(const QByteArray& state) -> void override;

        /**
         * @brief Returns the last saved main window window state.
         * @return The window state as an integer.
         */
        [[nodiscard]] auto get_mainwindow_windowstate() -> int override;

        /**
         * @brief Sets the main window window state.
         * @param state The window state to save as an integer (e.g. Qt::WindowNoState,
         * Qt::WindowMinimized, etc.).
         */
        auto set_mainwindow_windowstate(int state) -> void override;

    signals:
        /**
         * @brief Emitted when the language is changed.
         * @param language_code The new language code.
         */
        void languageCodeChanged(const QString& language_code) override;

        /**
         * @brief Emitted when the language name is changed.
         * @param language_name The new language name.
         */
        void languageNameChanged(const QString& language_name) override;

        /**
         * @brief Emitted when the theme is changed.
         * @param theme_name The new theme name.
         */
        void themeChanged(const QString& theme_name) override;
};
