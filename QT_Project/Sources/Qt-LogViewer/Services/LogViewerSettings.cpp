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
auto LogViewerSettings::get_language_code() -> QString
{
    return get_value("General", "language_code", "en").toString();
}

/**
 * @brief Sets the language code.
 * @param value The language code to set (e.g. "en", "de").
 */
auto LogViewerSettings::set_language_code(const QString& language_code) -> void
{
    set_value("General", "language_code", language_code);
    emit languageCodeChanged(language_code);
}

/**
 * @brief Returns the current language name (e.g. "English", "Deutsch").
 */
auto LogViewerSettings::get_language_name() -> QString
{
    return get_value("General", "language_name", "English").toString();
}

/**
 * @brief Sets the language name.
 * @param language_name The language name to set (e.g. "English", "Deutsch").
 */
auto LogViewerSettings::set_language_name(const QString& language_name) -> void
{
    set_value("General", "language_name", language_name);
    emit languageNameChanged(language_name);
}

/**
 * @brief Returns the last saved main window geometry (position and size).
 * @return The geometry data as QByteArray.
 */
auto LogViewerSettings::get_mainwindow_geometry() -> QByteArray
{
    return get_value("MainWindow", "geometry", QByteArray()).toByteArray();
}

/**
 * @brief Sets the main window geometry (position and size).
 * @param geometry The geometry data to save as QByteArray.
 */
auto LogViewerSettings::set_mainwindow_geometry(const QByteArray& geometry) -> void
{
    set_value("MainWindow", "geometry", geometry);
}

/**
 * @brief Returns the last saved main window state.
 * @return The state data as a QByteArray.
 */
auto LogViewerSettings::get_mainwindow_state() -> QByteArray
{
    return get_value("MainWindow", "state", QByteArray()).toByteArray();
}

/**
 * @brief Sets the main window state.
 * @param state The state data to save as a QByteArray.
 */
auto LogViewerSettings::set_mainwindow_state(const QByteArray& state) -> void
{
    set_value("MainWindow", "state", state);
}

/**
 * @brief Returns the last saved main window window state.
 * @return The window state as an integer.
 */
auto LogViewerSettings::get_mainwindow_windowstate() -> int
{
    // 0 = normal, 1 = minimized, 2 = maximized
    return get_value("MainWindow", "windowState", Qt::WindowNoState).toInt();
}

/**
 * @brief Sets the main window window state.
 * @param state The window state to save as an integer (e.g. Qt::WindowNoState, Qt::WindowMinimized,
 * etc.).
 */
auto LogViewerSettings::set_mainwindow_windowstate(int state) -> void
{
    set_value("MainWindow", "windowState", state);
}
