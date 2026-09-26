#pragma once

#include <QVector>

#include "Qt-LogViewer/Models/LogFieldDefinition.h"
#include "Qt-LogViewer/Services/LogParsingProfile.h"

/**
 * @file LogColumnSchemaBuilder.h
 * @brief Declares the builder for deterministic log-model column schemas.
 */

/**
 * @class LogColumnSchemaBuilder
 * @brief Converts parsing profiles into one ordered, duplicate-free column schema.
 *
 * The builder keeps parser-specific field discovery outside controllers and models. Built-in
 * fields retain their capabilities, while custom fields receive readable display names.
 */
class LogColumnSchemaBuilder final
{
    public:
        LogColumnSchemaBuilder() = delete;

        /**
         * @brief Builds the display columns supplied by one parsing profile.
         * @param profile Profile whose resolved field order defines the schema.
         * @return Ordered, duplicate-free column definitions.
         */
        [[nodiscard]] static auto build(const LogParsingProfile& profile)
            -> QVector<LogFieldDefinition>;

        /**
         * @brief Merges the display columns supplied by multiple parsing profiles.
         * @param profiles Profiles in the order in which their fields should be considered.
         * @return Ordered schema containing each field identifier once.
         */
        [[nodiscard]] static auto build(const QVector<LogParsingProfile>& profiles)
            -> QVector<LogFieldDefinition>;
};
