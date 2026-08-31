#include "Qt-LogViewer/Services/LogViewerSettings.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <optional>
#include <utility>

namespace
{
constexpr int k_parsing_profiles_schema_version = 1;
constexpr auto k_parsing_profiles_group = "ParsingProfiles";
constexpr auto k_parsing_profiles_key = "profiles";
}  // namespace

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

/**
 * @brief Loads the globally available log parsing profiles.
 *
 * The complete collection is decoded as one versioned JSON document. A malformed entry rejects
 * the document so callers never receive a silently incomplete profile list.
 *
 * @param error_message Optional destination for stored-data decoding errors.
 * @return Stored profiles, or an empty vector when none exist or decoding fails.
 */
auto LogViewerSettings::get_log_parsing_profiles(QString* error_message)
    -> QVector<LogParsingProfile>
{
    QVector<LogParsingProfile> profiles;
    QString error;
    const QByteArray content = get_value(QString::fromLatin1(k_parsing_profiles_group),
                                         QString::fromLatin1(k_parsing_profiles_key), QByteArray())
                                   .toByteArray();

    if (!content.isEmpty())
    {
        QJsonParseError parse_error;
        const QJsonDocument document = QJsonDocument::fromJson(content, &parse_error);

        if (parse_error.error != QJsonParseError::NoError || !document.isObject())
        {
            error =
                parse_error.error != QJsonParseError::NoError
                    ? parse_error.errorString()
                    : QStringLiteral("The parsing profile settings must contain a JSON object.");
        }

        const QJsonObject root = document.object();
        if (error.isEmpty() && root.value(QStringLiteral("schema_version")).toInt(-1) !=
                                   k_parsing_profiles_schema_version)
        {
            error = QStringLiteral("Unsupported parsing profile settings schema version.");
        }

        const QJsonValue profiles_value = root.value(QStringLiteral("profiles"));
        if (error.isEmpty() && !profiles_value.isArray())
        {
            error = QStringLiteral("The parsing profile settings require a profiles array.");
        }

        if (error.isEmpty())
        {
            const QJsonArray serialized_profiles = profiles_value.toArray();

            for (qsizetype index = 0; index < serialized_profiles.size(); ++index)
            {
                const QJsonValue profile_value = serialized_profiles.at(index);

                if (error.isEmpty() && !profile_value.isObject())
                {
                    error =
                        QStringLiteral("Parsing profile at index %1 must be an object.").arg(index);
                }

                if (error.isEmpty())
                {
                    QString profile_error;
                    std::optional<LogParsingProfile> profile =
                        LogParsingProfile::from_json(profile_value.toObject(), &profile_error);

                    if (profile.has_value())
                    {
                        profiles.append(std::move(profile.value()));
                    }
                    else
                    {
                        error = QStringLiteral("Invalid parsing profile at index %1: %2")
                                    .arg(index)
                                    .arg(profile_error);
                    }
                }
            }
        }
    }

    if (!error.isEmpty())
    {
        profiles.clear();
    }

    if (error_message != nullptr)
    {
        *error_message = error;
    }

    return profiles;
}

/**
 * @brief Replaces the globally stored log parsing profiles.
 *
 * Profiles use their canonical JSON representation and are stored as one compact, versioned
 * document in the existing QSettings backend.
 *
 * @param profiles Complete profile collection to persist in the application settings.
 * @return True when QSettings synchronized the collection successfully.
 */
auto LogViewerSettings::set_log_parsing_profiles(const QVector<LogParsingProfile>& profiles) -> bool
{
    QJsonArray serialized_profiles;
    for (const LogParsingProfile& profile: profiles)
    {
        serialized_profiles.append(profile.to_json());
    }

    QJsonObject root;
    root.insert(QStringLiteral("schema_version"), k_parsing_profiles_schema_version);
    root.insert(QStringLiteral("profiles"), serialized_profiles);

    set_value(QString::fromLatin1(k_parsing_profiles_group),
              QString::fromLatin1(k_parsing_profiles_key),
              QJsonDocument(root).toJson(QJsonDocument::Compact));
    sync();
    return status() == QSettings::NoError;
}
