#pragma once

#include <QString>
#include <QStringView>
#include <optional>

#include "SimpleCppLogger/LogLevel.h"

/**
 * @file LogLevelMapping.h
 * @brief Declares Qt-friendly conversion between textual and canonical log levels.
 */

/**
 * @brief Provides stateless conversion helpers for supported log-level names and aliases.
 */
namespace LogLevels
{
/**
 * @brief Resolves a textual log level, including supported aliases and decorated names.
 * @param text Log-level text to classify.
 * @return Canonical log level, or std::nullopt when the text is unknown.
 */
[[nodiscard]] auto from_text(QStringView text) -> std::optional<SimpleCppLogger::LogLevel>;

/**
 * @brief Converts a canonical log level to its Qt text representation.
 * @param level Canonical log level.
 * @return Canonical display text, or an empty string for an invalid enum value.
 */
[[nodiscard]] auto to_text(SimpleCppLogger::LogLevel level) -> QString;
}  // namespace LogLevels
