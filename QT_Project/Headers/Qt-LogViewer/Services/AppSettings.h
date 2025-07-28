#pragma once

#include <QString>

#include "Qt-LogViewer/Services/Settings.h"

/**
 * @class AppSettings
 * @brief Application-specific settings interface with type-safe getters/setters.
 *
 * Provides convenience methods for common application settings such as theme and language.
 * Inherits from Settings and can be extended for further application-specific settings.
 */
class AppSettings: public Settings
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs an AppSettings object with the given parent.
         *        Uses QCoreApplication::organizationName() and applicationName() for QSettings.
         * @param parent The parent QObject, or nullptr.
         */
        explicit AppSettings(QObject* parent = nullptr);

        ~AppSettings() override = default;

        /**
         * @brief Returns the current theme.
         * @return The theme name (e.g. "Dark", "Light"). Default is "Dark".
         */
        [[nodiscard]] auto get_theme() const -> QString;

        /**
         * @brief Sets the theme.
         * @param value The theme name to set (e.g. "Dark", "Light").
         */
        auto set_theme(const QString& value) -> void;

        /**
         * @brief Returns the current language code.
         * @return The language code (e.g. "en", "de"). Default is "en".
         */
        [[nodiscard]] auto get_language() const -> QString;

        /**
         * @brief Sets the language code.
         * @param value The language code to set (e.g. "en", "de").
         */
        auto set_language(const QString& value) -> void;
};
