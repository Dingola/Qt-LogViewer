#pragma once

#include <QObject>
#include <QSettings>
#include <QStringList>
#include <QVariant>

/**
 * @class Settings
 * @brief Wrapper class for QSettings.
 *
 * Encapsulates basic methods for reading, writing, and managing application settings
 * using QSettings.
 * Can be used directly or as a base class for application-specific settings.
 */
class Settings: public QObject
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs a Settings object with the given parent.
         *        Uses QCoreApplication::organizationName() and applicationName() for QSettings.
         * @param parent The parent QObject, or nullptr.
         */
        explicit Settings(QObject* parent = nullptr);

        ~Settings() override = default;

        // NOLINTBEGIN(modernize-use-trailing-return-type)

        /**
         * @brief Gets the value for a key.
         * @param key The key to look up.
         * @param default_value The value to return if the key does not exist.
         * @return The value for the key, or default_value if not found.
         */
        [[nodiscard]] Q_INVOKABLE QVariant
        get_value(const QString& key, const QVariant& default_value = QVariant()) const;

        /**
         * @brief Gets the value for a key in a group.
         * @param group The group name.
         * @param key The key name.
         * @param default_value The value to return if the key does not exist.
         * @return The value for the key, or default_value if not found.
         */
        [[nodiscard]] Q_INVOKABLE QVariant get_value(const QString& group, const QString& key,
                                                     const QVariant& default_value) const;

        /**
         * @brief Sets the value for a key.
         * @param key The key to set.
         * @param value The value to set.
         */
        Q_INVOKABLE void set_value(const QString& key, const QVariant& value);

        /**
         * @brief Sets the value for a key in a group.
         * @param group The group name.
         * @param key The key name.
         * @param value The value to set.
         */
        Q_INVOKABLE void set_value(const QString& group, const QString& key, const QVariant& value);

        /**
         * @brief Returns a list of all child groups.
         * @return List of group names.
         */
        [[nodiscard]] Q_INVOKABLE QStringList child_groups() const;

        /**
         * @brief Returns a list of child groups in a group.
         * @param group The group name.
         * @return List of group names.
         */
        [[nodiscard]] Q_INVOKABLE QStringList child_groups(const QString& group) const;

        /**
         * @brief Returns a list of child keys in a group.
         * @param group The group name.
         * @return List of key names.
         */
        [[nodiscard]] Q_INVOKABLE QStringList child_keys(const QString& group) const;

        /**
         * @brief Returns a list of all keys.
         * @return List of all keys.
         */
        [[nodiscard]] Q_INVOKABLE QStringList all_keys() const;

        /**
         * @brief Checks if a key exists.
         * @param key The key to check.
         * @return True if the key exists, false otherwise.
         */
        [[nodiscard]] Q_INVOKABLE bool contains(const QString& key) const;

        /**
         * @brief Checks if a key exists in a group.
         * @param group The group name.
         * @param key The key name.
         * @return True if the key exists, false otherwise.
         */
        [[nodiscard]] Q_INVOKABLE bool contains(const QString& group, const QString& key) const;

        /**
         * @brief Loads settings from a file (overwrites current settings).
         * @param file_path The file to load from.
         * @param format The QSettings format (default: IniFormat).
         */
        Q_INVOKABLE void load_from_file(const QString& file_path,
                                        QSettings::Format format = QSettings::IniFormat);

        /**
         * @brief Saves current settings to a file.
         * @param file_path The file to save to.
         * @param format The QSettings format (default: IniFormat).
         */
        Q_INVOKABLE void save_to_file(const QString& file_path,
                                      QSettings::Format format = QSettings::IniFormat);

        /**
         * @brief Clears all settings.
         */
        Q_INVOKABLE void clear();

        // NOLINTEND(modernize-use-trailing-return-type)

    protected:
        /**
         * @brief Copies all settings from source to destination (recursive).
         * @param source The source QSettings.
         * @param destination The destination QSettings.
         */
        auto copy_settings(QSettings& source, QSettings& destination) -> void;

    private:
        mutable QSettings m_settings;
};
