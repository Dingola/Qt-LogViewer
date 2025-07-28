#include "Qt-LogViewer/Services/Settings.h"

#include <QCoreApplication>
#include <QDebug>

/**
 * @brief Constructs an Settings object with the given parent.
 *
 * This constructor initializes the Settings object with the parent object and
 * sets up the QSettings object to use the organization name and application name
 * for storing settings in the registry. The settings can be cleared using the
 * `clear()` method.
 *
 * @param parent The parent object.
 */
Settings::Settings(QObject* parent)
    : QObject(parent),
      m_settings(QCoreApplication::organizationName(), QCoreApplication::applicationName())
{
    qInfo() << "Settings initialized with organization name:"
            << QCoreApplication::organizationName()
            << "and application name:" << QCoreApplication::applicationName()
            << ". Session settings will be saved to:" << m_settings.fileName();
}

// NOLINTBEGIN(modernize-use-trailing-return-type)

/**
 * @brief Gets the value associated with the specified key from the settings.
 *
 * @param key The key of the value to retrieve.
 * @param default_value The default value to return if the key does not exist.
 * @return The value associated with the key, or the default value if the key does not exist.
 */
QVariant Settings::get_value(const QString& key, const QVariant& default_value) const
{
    return m_settings.value(key, default_value);
}

/**
 * @brief Gets the value associated with the specified key from the settings.
 *
 * @param group The group of the value to retrieve.
 * @param key The key of the value to retrieve.
 * @param default_value The default value to return if the key does not exist.
 * @return The value associated with the key, or the default value if the key does not exist.
 */
QVariant Settings::get_value(const QString& group, const QString& key,
                             const QVariant& default_value) const
{
    m_settings.beginGroup(group);
    QVariant value = m_settings.value(key, default_value);
    m_settings.endGroup();
    return value;
}

/**
 * @brief Sets the value associated with the specified key in the settings.
 *
 * @param key The key of the value to set.
 * @param value The value to set.
 */
void Settings::set_value(const QString& key, const QVariant& value)
{
    m_settings.setValue(key, value);
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
    m_settings.beginGroup(group);
    m_settings.setValue(key, value);
    m_settings.endGroup();
}

/**
 * @brief Returns a list of child groups in the settings.
 *
 * @return A list of child groups in the settings.
 */
QStringList Settings::child_groups() const
{
    return m_settings.childGroups();
}

/**
 * @brief Returns a list of child groups in the specified group.
 *
 * @param group The group to retrieve the child groups from.
 * @return A list of child groups in the specified group.
 */
QStringList Settings::child_groups(const QString& group) const
{
    m_settings.beginGroup(group);
    QStringList groups = m_settings.childGroups();
    m_settings.endGroup();
    return groups;
}

/**
 * @brief Returns a list of child keys in the specified group.
 *
 * @param group The group to retrieve the child keys from.
 * @return A list of child keys in the specified group.
 */
QStringList Settings::child_keys(const QString& group) const
{
    m_settings.beginGroup(group);
    QStringList keys = m_settings.childKeys();
    m_settings.endGroup();
    return keys;
}

/**
 * @brief Returns a list of all keys in the settings.
 *
 * @return A list of all keys in the settings.
 */
QStringList Settings::all_keys() const
{
    return m_settings.allKeys();
}

/**
 * @brief Checks if the specified key exists in the settings.
 *
 * @param key The key to check.
 * @return True if the key exists, false otherwise.
 */
bool Settings::contains(const QString& key) const
{
    return m_settings.contains(key);
}

/**
 * @brief Checks if the specified key exists in the settings.
 *
 * @param group The group of the key.
 * @param key The key to check.
 * @return True if the key exists, false otherwise.
 */
bool Settings::contains(const QString& group, const QString& key) const
{
    m_settings.beginGroup(group);
    bool exists = m_settings.contains(key);
    m_settings.endGroup();
    return exists;
}

/**
 * @brief Loads the settings from the specified file.
 *
 * This function clears the existing settings and loads the settings from the specified file.
 *
 * @param file_path The file path of the settings file to load.
 * @param format The format of the settings file.
 */
void Settings::load_from_file(const QString& file_path, QSettings::Format format)
{
    qInfo() << "Loading settings from file:" << file_path;
    QSettings file_settings(file_path, format);
    clear();
    copy_settings(file_settings, m_settings);
    qDebug() << "Loaded settings from file:" << file_path
             << ". All loaded keys:" << m_settings.allKeys();
}

/**
 * @brief Saves the settings to the specified file.
 *
 * This function saves the current settings to the specified file.
 *
 * @param file_path The file path of the settings file to save.
 * @param format The format of the settings file.
 */
void Settings::save_to_file(const QString& file_path, QSettings::Format format)
{
    qInfo() << "Saving settings to file:" << file_path;
    QSettings file_settings(file_path, format);
    copy_settings(m_settings, file_settings);
    qDebug() << "Saved settings to file:" << file_path
             << ". All saved keys:" << file_settings.allKeys();
}

/**
 * @brief Clears the current session settings.
 *
 * This function clears the current session settings.
 */
void Settings::clear()
{
    qInfo() << "Clearing current session settings";
    m_settings.clear();
    m_settings.sync();
}

/**
 * @brief Copies the settings from the source QSettings object to the destination QSettings object.
 *
 * This function recursively copies all the settings, including child groups and keys, from the
 * source QSettings object to the destination QSettings object.
 *
 * @param source The source QSettings object.
 * @param destination The destination QSettings object.
 */
auto Settings::copy_settings(QSettings& source, QSettings& destination) -> void
{
    for (const auto& child_key: source.childKeys())
    {
        destination.setValue(child_key, source.value(child_key));
    }

    for (const auto& child_group: source.childGroups())
    {
        source.beginGroup(child_group);
        destination.beginGroup(child_group);

        copy_settings(source, destination);

        source.endGroup();
        destination.endGroup();
    }
}

// NOLINTEND(modernize-use-trailing-return-type)
