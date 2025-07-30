#pragma once

#include <QObject>
#include <QSettings>
#include <QStandardPaths>
#include <QStringList>
#include <QVariant>

/**
 * @class Settings
 * @brief A class for managing application settings with a simple interface.
 *
 * This class extends QSettings to provide a more convenient API for getting and setting
 * values in groups, checking for key existence, and listing child groups and keys.
 */
class Settings: public QSettings
{
        Q_OBJECT

    public:
        using QSettings::QSettings;

        ~Settings() override = default;

        // NOLINTBEGIN(modernize-use-trailing-return-type)

        /**
         * @brief Gets the value for a key in a group.
         * @param group The group name.
         * @param key The key name.
         * @param default_value The value to return if the key does not exist.
         * @return The value for the key, or default_value if not found.
         */
        [[nodiscard]] Q_INVOKABLE QVariant get_value(const QString& group, const QString& key,
                                                     const QVariant& default_value);

        /**
         * @brief Sets the value for a key in a group.
         * @param group The group name.
         * @param key The key name.
         * @param value The value to set.
         */
        Q_INVOKABLE void set_value(const QString& group, const QString& key, const QVariant& value);

        /**
         * @brief Returns a list of child groups in a group.
         * @param group The group name.
         * @return List of group names.
         */
        [[nodiscard]] Q_INVOKABLE QStringList child_groups(const QString& group);

        /**
         * @brief Returns a list of child keys in a group.
         * @param group The group name.
         * @return List of key names.
         */
        [[nodiscard]] Q_INVOKABLE QStringList child_keys(const QString& group);

        /**
         * @brief Checks if a key exists in a group.
         * @param group The group name.
         * @param key The key name.
         * @return True if the key exists, false otherwise.
         */
        [[nodiscard]] Q_INVOKABLE bool contains(const QString& group, const QString& key);

        // NOLINTEND(modernize-use-trailing-return-type)

        /**
         * @brief Returns a settings file path in the given standard location with the specified
         * file name.
         *
         * Ensures the directory exists and returns the full path.
         *
         * @param location The QStandardPaths::StandardLocation (e.g. AppConfigLocation).
         * @param file_name The name of the settings file (e.g. "settings.ini").
         * @return The full file path as a QString.
         */
        [[nodiscard]] static auto settings_file_path(QStandardPaths::StandardLocation location,
                                                     const QString& file_name) -> QString;

        /**
         * @brief Returns the default settings file path ("settings.ini" in AppConfigLocation).
         *
         * Ensures the directory exists and returns the full path.
         *
         * @return The full file path as a QString.
         */
        [[nodiscard]] static auto default_settings_file_path() -> QString;
};
