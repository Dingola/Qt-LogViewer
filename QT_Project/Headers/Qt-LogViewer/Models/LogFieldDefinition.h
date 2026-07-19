#pragma once

#include <QString>
#include <QVector>

/**
 * @file LogFieldDefinition.h
 * @brief Defines stable log field identifiers and their capabilities.
 */

/**
 * @brief Describes the value type represented by a log field.
 */
enum class LogFieldValueType
{
    Text,
    Timestamp,
    Integer,
    FloatingPoint,
    Boolean
};

/**
 * @brief Describes one field supplied by the built-in schema or a log parser.
 */
struct LogFieldDefinition {
        QString id;
        QString display_name;
        LogFieldValueType value_type{LogFieldValueType::Text};
        bool searchable{false};
        bool filterable{false};
        bool sortable{false};
        bool visible{true};
};

/**
 * @brief Stable identifiers for the currently built-in log fields.
 */
namespace LogField
{
inline const QString Timestamp{QStringLiteral("timestamp")};

inline const QString Level{QStringLiteral("level")};

inline const QString Message{QStringLiteral("message")};

inline const QString AppName{QStringLiteral("app_name")};

inline const QString FilePath{QStringLiteral("file_path")};

inline const QString InsertionOrder{QStringLiteral("_insertion_order")};
}  // namespace LogField

/**
 * @brief Returns the definitions of the currently built-in log fields.
 * @return Built-in field definitions.
 */
[[nodiscard]] inline auto get_builtin_log_field_definitions() -> const QVector<LogFieldDefinition>&
{
    static const QVector<LogFieldDefinition> definitions{
        {LogField::Timestamp, QStringLiteral("Timestamp"), LogFieldValueType::Timestamp, false,
         false, true, true},
        {LogField::Level, QStringLiteral("Level"), LogFieldValueType::Text, true, true, true, true},
        {LogField::Message, QStringLiteral("Message"), LogFieldValueType::Text, true, false, true,
         true},
        {LogField::AppName, QStringLiteral("Application"), LogFieldValueType::Text, true, true,
         true, true},
        {LogField::FilePath, QStringLiteral("File"), LogFieldValueType::Text, true, true, true,
         true},
        {LogField::InsertionOrder, QStringLiteral("Insertion order"), LogFieldValueType::Integer,
         false, false, true, false}};

    return definitions;
}
