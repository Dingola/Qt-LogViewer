#include "Qt-LogViewer/Services/LogViewerSettings.h"

/**
 * @brief Returns the current theme.
 * @return The theme name (e.g. "Dark", "Light"). Default is "Dark".
 */
auto LogViewerSettings::get_theme() -> QString
{
    return get_value("Appearance", "theme", "Dark").toString();
}

/**
 * @brief Sets the theme.
 * @param value The theme name to set (e.g. "Dark", "Light").
 */
auto LogViewerSettings::set_theme(const QString& value) -> void
{
    set_value("Appearance", "theme", value);
    emit themeChanged(value);
}

/**
 * @brief Returns the current language code.
 * @return The language code (e.g. "en", "de"). Default is "en".
 */
auto LogViewerSettings::get_language() -> QString
{
    return get_value("General", "language", "en").toString();
}

/**
 * @brief Sets the language code.
 * @param value The language code to set (e.g. "en", "de").
 */
auto LogViewerSettings::set_language(const QString& value) -> void
{
    set_value("General", "language", value);
    emit languageChanged(value);
}
