#include "Qt-LogViewer/Services/Settings.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>

// NOLINTBEGIN(modernize-use-trailing-return-type)

/**
 * @brief Gets the value associated with the specified key from the settings.
 *
 * @param group The group of the value to retrieve.
 * @param key The key of the value to retrieve.
 * @param default_value The default value to return if the key does not exist.
 * @return The value associated with the key, or the default value if the key does not exist.
 */
QVariant Settings::get_value(const QString& group, const QString& key,
                             const QVariant& default_value)
{
    beginGroup(group);
    QVariant value = QSettings::value(key, default_value);
    endGroup();

    return value;
}

/**
 * @brief Sets the value associated with the specified key in the settings.
 *
 * @param group The group of the value to set.
 * @param key The key of the value to set.
 * @param value The value to set.
 */
void Settings::set_value(const QString& group, const QString& key, const QVariant& value)
{
    beginGroup(group);
    setValue(key, value);
    endGroup();
}

/**
 * @brief Returns a list of child groups in the specified group.
 *
 * @param group The group to retrieve the child groups from.
 * @return A list of child groups in the specified group.
 */
QStringList Settings::child_groups(const QString& group)
{
    beginGroup(group);
    QStringList groups = childGroups();
    endGroup();

    return groups;
}

/**
 * @brief Returns a list of child keys in the specified group.
 *
 * @param group The group to retrieve the child keys from.
 * @return A list of child keys in the specified group.
 */
QStringList Settings::child_keys(const QString& group)
{
    beginGroup(group);
    QStringList keys = childKeys();
    endGroup();

    return keys;
}

/**
 * @brief Checks if the specified key exists in the settings.
 *
 * @param group The group of the key.
 * @param key The key to check.
 * @return True if the key exists, false otherwise.
 */
bool Settings::contains(const QString& group, const QString& key)
{
    beginGroup(group);
    bool exists = QSettings::contains(key);
    endGroup();

    return exists;
}

// NOLINTEND(modernize-use-trailing-return-type)

/**
 * @brief Returns a settings file path in the given standard location with the specified file name.
 *
 * Ensures the directory exists and returns the full path.
 *
 * @param location The standard location to use (e.g., AppConfigLocation).
 * @param file_name The name of the settings file.
 * @return The full path to the settings file.
 */
QString Settings::settings_file_path(QStandardPaths::StandardLocation location,
                                     const QString& file_name)
{
    const QString dir = QStandardPaths::writableLocation(location);
    QDir().mkpath(dir);
    return dir + QDir::separator() + file_name;
}

/**
 * @brief Returns the default settings file path ("settings.ini" in AppConfigLocation).
 *
 * Ensures the directory exists and returns the full path.
 *
 * @return The full path to the default settings file.
 */
QString Settings::default_settings_file_path()
{
    return settings_file_path(QStandardPaths::AppConfigLocation, QStringLiteral("settings.ini"));
}
