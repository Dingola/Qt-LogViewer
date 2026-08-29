/**
 * @file LogParsingProfile.cpp
 * @brief Implements the log parsing profile value object.
 */

#include "Qt-LogViewer/Services/LogParsingProfile.h"

#include <QStringList>
#include <utility>

#include "Qt-LogViewer/Models/LogFieldDefinition.h"
#include "QtRecordParser/BuiltInConverters.h"
#include "QtRecordParser/FormatRecordParser.h"

/**
 * @brief Creates the standard LogViewer profile for a placeholder-based format.
 *
 * The resulting profile uses the requested display name, a generated stable identifier and the
 * built-in log field definitions.
 *
 * @param format_string Placeholder-based record format.
 * @param name Human-readable profile name.
 * @return Profile containing a generated identifier and the built-in log field definitions.
 */
auto LogParsingProfile::create_default(const QString& format_string,
                                       const QString& name) -> LogParsingProfile
{
    return LogParsingProfile(QUuid::createUuid(), name, create_log_configuration(format_string));
}

/**
 * @brief Constructs a profile from a complete parser configuration.
 *
 * A null identifier is replaced with a generated UUID. Missing built-in log fields are added
 * while explicit definitions remain authoritative. Invalid parser configurations are retained so
 * callers can inspect and present their validation errors.
 *
 * @param id Stable profile identifier; a new identifier is generated when null.
 * @param name Human-readable profile name.
 * @param configuration Serializable parser configuration.
 */
LogParsingProfile::LogParsingProfile(QUuid id, QString name,
                                     QtRecordParser::ParserConfiguration configuration)
    : m_id(id.isNull() ? QUuid::createUuid() : std::move(id)),
      m_name(std::move(name)),
      m_configuration(apply_log_defaults(std::move(configuration)))
{}

/**
 * @brief Returns the stable profile identifier.
 * @return Non-null identifier retained by derived profile copies.
 */
auto LogParsingProfile::get_id() const noexcept -> const QUuid&
{
    return m_id;
}

/**
 * @brief Returns the human-readable profile name.
 * @return Profile display name.
 */
auto LogParsingProfile::get_name() const noexcept -> const QString&
{
    return m_name;
}

/**
 * @brief Returns the normalized parser configuration.
 * @return Serializable configuration containing the built-in log field defaults.
 */
auto LogParsingProfile::get_configuration() const noexcept
    -> const QtRecordParser::ParserConfiguration&
{
    return m_configuration;
}

/**
 * @brief Creates a profile copy with replacement parser configuration.
 *
 * The replacement configuration is normalized while identity and display name stay unchanged.
 *
 * @param configuration Replacement parser configuration to normalize.
 * @return Profile copy with unchanged identifier and name.
 */
auto LogParsingProfile::with_configuration(QtRecordParser::ParserConfiguration configuration) const
    -> LogParsingProfile
{
    return LogParsingProfile(m_id, m_name, std::move(configuration));
}

/**
 * @brief Checks whether the profile can create a usable format parser.
 *
 * Validation resolves configured converter identifiers through the supplied registry.
 *
 * @param registry Converter registry used to validate configured converter identifiers.
 * @return True when format, fields, converters and generated pattern are valid.
 */
auto LogParsingProfile::is_valid(QtRecordParser::ConverterRegistry registry) const -> bool
{
    return get_validation_error(std::move(registry)).isEmpty();
}

/**
 * @brief Validates the profile through the same parser implementation used at runtime.
 *
 * Using the runtime parser here keeps validation semantics aligned with actual parsing. The
 * supplied registry resolves converter identifiers.
 *
 * @param registry Converter registry used to validate configured converter identifiers.
 * @return Human-readable validation error, or an empty string when valid.
 */
auto LogParsingProfile::get_validation_error(QtRecordParser::ConverterRegistry registry) const
    -> QString
{
    const QtRecordParser::FormatRecordParser parser(m_configuration, std::move(registry));
    return parser.get_configuration_error();
}

/**
 * @brief Creates the built-in LogViewer configuration for a format string.
 *
 * The configuration combines the placeholder-based format with standard log field definitions
 * and enables unknown fields for custom placeholders.
 *
 * @param format_string Placeholder-based record format.
 * @return Configuration containing all built-in log field definitions.
 */
auto LogParsingProfile::create_log_configuration(const QString& format_string)
    -> QtRecordParser::ParserConfiguration
{
    QtRecordParser::ParserConfiguration configuration;
    configuration.format = format_string;
    configuration.fields = get_default_log_fields();
    configuration.allow_unknown_fields = true;
    return configuration;
}

/**
 * @brief Adds missing built-in fields without replacing explicit configurations.
 *
 * Built-in fields may be absent from the format and are still retained in the configuration so a
 * later profile editor can enable them without reconstructing their converter defaults.
 *
 * The normalized configuration contains every built-in log field exactly once.
 *
 * @param configuration User-, file- or generated parser configuration.
 * @return Normalized configuration containing every built-in log field definition.
 */
auto LogParsingProfile::apply_log_defaults(QtRecordParser::ParserConfiguration configuration)
    -> QtRecordParser::ParserConfiguration
{
    const QVector<QtRecordParser::FieldConfiguration> defaults = get_default_log_fields();

    for (const QtRecordParser::FieldConfiguration& default_field: defaults)
    {
        bool field_exists = false;

        for (const QtRecordParser::FieldConfiguration& configured_field: configuration.fields)
        {
            if (!field_exists && configured_field.id == default_field.id)
            {
                field_exists = true;
            }
        }

        if (!field_exists)
        {
            configuration.fields.append(default_field);
        }
    }

    return configuration;
}

/**
 * @brief Returns the standard LogViewer field definitions.
 *
 * The timestamp field accepts ISO-8601 plus the common date-time formats historically supported
 * by LogParser. File, line and function remain optional source-oriented fields, while unknown
 * placeholders are handled generically by QtRecordParser.
 *
 * @return Built-in field patterns, converter identifiers and converter options.
 */
auto LogParsingProfile::get_default_log_fields() -> QVector<QtRecordParser::FieldConfiguration>
{
    const QString timestamp_pattern = QStringLiteral(
        R"(\d{4}-\d{2}-\d{2}[ T]\d{2}:\d{2}:\d{2}(?:\.\d{1,3})?(?:Z|[+\-]\d{2}:\d{2})?|\d{2}\.\d{2}\.\d{4}\s+\d{2}:\d{2}:\d{2}(?:\.\d{1,3})?|\d{2}/\d{2}/\d{4}\s+\d{2}:\d{2}:\d{2}(?:\.\d{1,3})?|\d{4}/\d{2}/\d{2}\s+\d{2}:\d{2}:\d{2}(?:\.\d{1,3})?)");

    const QStringList timestamp_formats{
        QStringLiteral("yyyy-MM-dd HH:mm:ss"), QStringLiteral("yyyy-MM-dd HH:mm:ss.zzz"),
        QStringLiteral("yyyy-MM-ddTHH:mm:ss"), QStringLiteral("yyyy-MM-ddTHH:mm:ss.zzz"),
        QStringLiteral("dd.MM.yyyy HH:mm:ss"), QStringLiteral("dd.MM.yyyy HH:mm:ss.zzz"),
        QStringLiteral("MM/dd/yyyy HH:mm:ss"), QStringLiteral("MM/dd/yyyy HH:mm:ss.zzz"),
        QStringLiteral("yyyy/MM/dd HH:mm:ss"), QStringLiteral("yyyy/MM/dd HH:mm:ss.zzz")};

    QVariantMap timestamp_options;
    timestamp_options.insert(QStringLiteral("accept_iso"), true);
    timestamp_options.insert(QStringLiteral("formats"), timestamp_formats);

    return {{LogField::Timestamp, QStringLiteral("Timestamp"), timestamp_pattern,
             QtRecordParser::ConverterId::DateTime, timestamp_options, true},
            {LogField::Level, QStringLiteral("Level"), QStringLiteral(R"(\w+)"),
             QtRecordParser::ConverterId::Text, QVariantMap(), true},
            {LogField::Message, QStringLiteral("Message"), QStringLiteral(".*?"),
             QtRecordParser::ConverterId::Text, QVariantMap(), true},
            {LogField::AppName, QStringLiteral("Application"), QStringLiteral(R"(\S+)"),
             QtRecordParser::ConverterId::Text, QVariantMap(), true},
            {QStringLiteral("file"), QStringLiteral("File"), QStringLiteral(".*?"),
             QtRecordParser::ConverterId::Text, QVariantMap(), true},
            {QStringLiteral("line"), QStringLiteral("Line"), QStringLiteral(R"([+-]?\d+)"),
             QtRecordParser::ConverterId::Integer, QVariantMap(), true},
            {QStringLiteral("function"), QStringLiteral("Function"), QStringLiteral(".*?"),
             QtRecordParser::ConverterId::Text, QVariantMap(), true}};
}
