#include "Qt-LogViewer/Services/AppSettings.h"

/**
 * @brief Constructs an AppSettings object with the given parent.
 *        Uses QCoreApplication::organizationName() and applicationName() for QSettings.
 * @param parent The parent QObject, or nullptr.
 */
AppSettings::AppSettings(QObject* parent): Settings(parent) {}

/**
 * @brief Returns the current theme.
 * @return The theme name (e.g. "Dark", "Light"). Default is "Dark".
 */
auto AppSettings::get_theme() const -> QString
{
    return get_value("Appearance", "theme", "Dark").toString();
}

/**
 * @brief Sets the theme.
 * @param value The theme name to set (e.g. "Dark", "Light").
 */
auto AppSettings::set_theme(const QString& value) -> void
{
    set_value("Appearance", "theme", value);
}

/**
 * @brief Returns the current language code.
 * @return The language code (e.g. "en", "de"). Default is "en".
 */
auto AppSettings::get_language() const -> QString
{
    return get_value("General", "language", "en").toString();
}

/**
 * @brief Sets the language code.
 * @param value The language code to set (e.g. "en", "de").
 */
auto AppSettings::set_language(const QString& value) -> void
{
    set_value("General", "language", value);
}
