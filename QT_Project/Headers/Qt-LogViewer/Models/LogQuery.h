#pragma once

#include <QSet>
#include <QString>
#include <QUuid>
#include <Qt>

#include "Qt-LogViewer/Models/LogFieldDefinition.h"

/**
 * @file LogQuery.h
 * @brief Defines the query contract for database-backed log retrieval.
 */

/**
 * @brief Complete filter and sorting state for database-backed log retrieval.
 *
 * Pagination is intentionally kept outside this structure. LogQuery describes
 * the complete result set, while offset and limit select a page from it.
 *
 * Search and sort fields use stable string identifiers so fields supplied by
 * custom log parsers can later participate without changing this type.
 */
struct LogQuery {
        /**
         * @brief View whose archived entries are queried.
         */
        QUuid view_id;

        /**
         * @brief Selected application name, or empty for all applications.
         */
        QString app_name;

        /**
         * @brief Included log levels, or empty for all levels.
         */
        QSet<QString> log_levels;

        /**
         * @brief Plain-text or regular-expression search text.
         */
        QString search_text;

        /**
         * @brief Fields included in text searching.
         *
         * An empty set means all fields registered as searchable.
         */
        QSet<QString> search_fields;

        /**
         * @brief Whether search_text is interpreted as a regular expression.
         */
        bool use_regex{false};

        /**
         * @brief File exclusively included in the result, or empty when disabled.
         */
        QString show_only_file;

        /**
         * @brief Files excluded from the result.
         */
        QSet<QString> hidden_files;

        /**
         * @brief Stable identifier of the field used for sorting.
         *
         * The database layer must validate the identifier against its registered
         * sortable fields before constructing an SQL ORDER BY expression.
         */
        QString sort_field{LogField::Timestamp};

        /**
         * @brief Sort direction. The default presents newest timestamps first.
         */
        Qt::SortOrder sort_order{Qt::DescendingOrder};
};
