/**
 * @file LogColumnSchemaBuilder.cpp
 * @brief Implements parsing-profile to log-column schema conversion.
 */

#include "Qt-LogViewer/Models/LogColumnSchemaBuilder.h"

#include <QSet>

#include "Qt-LogViewer/Services/LogParser.h"

namespace
{
/**
 * @brief Resolves the complete definition for one parser field.
 * @param field_id Stable field identifier.
 * @param profile Profile containing optional display metadata.
 * @return Built-in capabilities or a readable custom-field definition.
 */
[[nodiscard]] auto resolve_definition(const QString& field_id,
                                      const LogParsingProfile& profile) -> LogFieldDefinition
{
    LogFieldDefinition definition;
    definition.id = field_id;
    definition.display_name = field_id;

    for (const LogFieldDefinition& builtin: get_builtin_log_field_definitions())
    {
        if (builtin.id == field_id)
        {
            definition = builtin;
            break;
        }
    }

    for (const QtRecordParser::FieldConfiguration& field: profile.get_configuration().fields)
    {
        if (field.id == field_id && !field.display_name.isEmpty())
        {
            definition.display_name = field.display_name;
            break;
        }
    }

    if (definition.display_name == field_id)
    {
        definition.display_name.replace(QLatin1Char('_'), QLatin1Char(' '));

        if (!definition.display_name.isEmpty())
        {
            definition.display_name[0] = definition.display_name.at(0).toUpper();
        }
    }

    return definition;
}
}  // namespace

/**
 * @brief Builds the display columns supplied by one parsing profile.
 * @param profile Profile whose resolved field order defines the schema.
 * @return Ordered, duplicate-free column definitions.
 */
auto LogColumnSchemaBuilder::build(const LogParsingProfile& profile) -> QVector<LogFieldDefinition>
{
    return build(QVector<LogParsingProfile>{profile});
}

/**
 * @brief Merges the display columns supplied by multiple parsing profiles.
 * @param profiles Profiles in stable precedence order.
 * @return Ordered schema containing each field identifier once.
 */
auto LogColumnSchemaBuilder::build(const QVector<LogParsingProfile>& profiles)
    -> QVector<LogFieldDefinition>
{
    QVector<LogFieldDefinition> columns;
    QSet<QString> included_fields;

    for (const LogParsingProfile& profile: profiles)
    {
        const QVector<QString> field_order = LogParser(profile).get_field_order().fields;

        for (const QString& field_id: field_order)
        {
            if (!included_fields.contains(field_id))
            {
                included_fields.insert(field_id);
                columns.append(resolve_definition(field_id, profile));
            }
        }
    }

    return columns;
}
