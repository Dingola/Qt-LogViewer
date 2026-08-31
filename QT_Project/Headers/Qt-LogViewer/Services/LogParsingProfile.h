#pragma once

#include <QJsonObject>
#include <QString>
#include <QUuid>
#include <QVector>
#include <optional>

#include "QtRecordParser/ConverterRegistry.h"
#include "QtRecordParser/ParserConfiguration.h"

/**
 * @file LogParsingProfile.h
 * @brief Declares the validatable value object describing one log parsing profile.
 */

/**
 * @class LogParsingProfile
 * @brief Owns the stable identity, display name and parser configuration of a log format.
 *
 * Profiles use value semantics and contain only serializable configuration data. Converter
 * implementations remain an injected runtime dependency of LogParser.
 */
class LogParsingProfile final
{
    public:
        /** Schema version used when parsing-profile persistence is introduced. */
        static constexpr int SchemaVersion = 1;

        /**
         * @brief Creates the built-in log profile for a format string.
         * @param format_string Placeholder-based record format.
         * @param name Human-readable profile name.
         * @return Profile containing the standard log field definitions.
         */
        [[nodiscard]] static auto create_default(const QString& format_string,
                                                 const QString& name = QStringLiteral("Default"))
            -> LogParsingProfile;

        /**
         * @brief Restores a parsing profile from its JSON representation.
         * @param object Serialized profile containing schema, identity, name and configuration.
         * @param error_message Optional destination for a decoding error.
         * @return Decoded profile, or std::nullopt when the representation is invalid.
         */
        [[nodiscard]] static auto from_json(const QJsonObject& object,
                                            QString* error_message = nullptr)
            -> std::optional<LogParsingProfile>;

        /**
         * @brief Creates a profile from a complete parser configuration.
         *
         * Missing built-in log field definitions are added without replacing explicitly
         * configured fields. Construction deliberately permits invalid parser configurations so
         * an editor or import preview can display get_validation_error().
         *
         * @param id Stable profile identifier; a new identifier is generated when null.
         * @param name Human-readable profile name.
         * @param configuration Serializable parser configuration.
         */
        LogParsingProfile(QUuid id, QString name,
                          QtRecordParser::ParserConfiguration configuration);

        /**
         * @brief Returns the stable profile identifier.
         * @return Non-null identifier retained by derived profile copies.
         */
        [[nodiscard]] auto get_id() const noexcept -> const QUuid&;

        /**
         * @brief Returns the human-readable profile name.
         * @return Profile display name.
         */
        [[nodiscard]] auto get_name() const noexcept -> const QString&;

        /**
         * @brief Returns the serializable parser configuration.
         * @return Configuration including built-in log field defaults.
         */
        [[nodiscard]] auto get_configuration() const noexcept
            -> const QtRecordParser::ParserConfiguration&;

        /**
         * @brief Converts this profile to its versioned JSON representation.
         * @return Serializable object containing schema, identity, name and configuration.
         */
        [[nodiscard]] auto to_json() const -> QJsonObject;

        /**
         * @brief Creates a copy with a replacement configuration and the same identity.
         * @param configuration Replacement parser configuration to normalize.
         * @return Profile copy with unchanged identifier and name.
         */
        [[nodiscard]] auto with_configuration(
            QtRecordParser::ParserConfiguration configuration) const -> LogParsingProfile;

        /**
         * @brief Returns whether the profile can construct a usable format parser.
         * @param registry Converter registry used to validate configured converter identifiers.
         * @return True when format, fields, converters and generated pattern are valid.
         */
        [[nodiscard]] auto is_valid(QtRecordParser::ConverterRegistry registry =
                                        QtRecordParser::ConverterRegistry::create_default()) const
            -> bool;

        /**
         * @brief Returns the parser validation error, or an empty string when valid.
         * @param registry Converter registry used to validate configured converter identifiers.
         * @return Human-readable validation error or an empty string.
         */
        [[nodiscard]] auto get_validation_error(
            QtRecordParser::ConverterRegistry registry =
                QtRecordParser::ConverterRegistry::create_default()) const -> QString;

    private:
        /**
         * @brief Creates the standard LogViewer parser configuration for a format string.
         * @param format_string Placeholder-based record format.
         * @return Configuration containing all built-in log field definitions.
         */
        [[nodiscard]] static auto create_log_configuration(const QString& format_string)
            -> QtRecordParser::ParserConfiguration;

        /**
         * @brief Adds missing built-in log fields without replacing explicit definitions.
         * @param configuration User-, file- or generated parser configuration.
         * @return Normalized configuration containing every built-in log field definition.
         */
        [[nodiscard]] static auto apply_log_defaults(
            QtRecordParser::ParserConfiguration configuration)
            -> QtRecordParser::ParserConfiguration;

        /**
         * @brief Returns standard field patterns, converters and timestamp options.
         * @return Built-in field definitions understood by the LogViewer adapter.
         */
        [[nodiscard]] static auto get_default_log_fields()
            -> QVector<QtRecordParser::FieldConfiguration>;

    private:
        /** Stable identifier used to reference the profile across sessions. */
        QUuid m_id;

        /** Human-readable name presented by profile-selection UIs. */
        QString m_name;

        /** Normalized, serializable configuration consumed by QtRecordParser. */
        QtRecordParser::ParserConfiguration m_configuration;
};
