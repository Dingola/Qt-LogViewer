/**
 * @file LogLevelMapping.cpp
 * @brief Implements Qt-friendly conversion between textual and canonical log levels.
 */

#include "Qt-LogViewer/Models/LogLevelMapping.h"

#include <string_view>

/**
 * @brief Resolves a textual log level, including supported aliases and decorated names.
 * @param text Log-level text to classify.
 * @return Canonical log level, or std::nullopt when the text is unknown.
 */
auto LogLevels::from_text(QStringView text) -> std::optional<SimpleCppLogger::LogLevel>
{
    const QString normalized = text.trimmed().toString().toCaseFolded();
    std::optional<SimpleCppLogger::LogLevel> result;

    if (normalized.contains(QStringLiteral("fatal")))
    {
        result = SimpleCppLogger::LogLevel::Fatal;
    }
    else if (normalized.contains(QStringLiteral("critical")))
    {
        result = SimpleCppLogger::LogLevel::Error;
    }
    else if (normalized.contains(QStringLiteral("error")) || normalized == QStringLiteral("err"))
    {
        result = SimpleCppLogger::LogLevel::Error;
    }
    else if (normalized.contains(QStringLiteral("warn")))
    {
        result = SimpleCppLogger::LogLevel::Warning;
    }
    else if (normalized.contains(QStringLiteral("debug")))
    {
        result = SimpleCppLogger::LogLevel::Debug;
    }
    else if (normalized.contains(QStringLiteral("trace")))
    {
        result = SimpleCppLogger::LogLevel::Trace;
    }
    else if (normalized.contains(QStringLiteral("info")))
    {
        result = SimpleCppLogger::LogLevel::Info;
    }

    return result;
}

/**
 * @brief Converts a canonical log level to its Qt text representation.
 * @param level Canonical log level.
 * @return Canonical display text, or an empty string for an invalid enum value.
 */
auto LogLevels::to_text(SimpleCppLogger::LogLevel level) -> QString
{
    const std::string_view text = SimpleCppLogger::to_string_view(level);
    const QString result = QString::fromLatin1(text.data(), static_cast<qsizetype>(text.size()));
    return result;
}
