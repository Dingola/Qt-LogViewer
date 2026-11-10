#pragma once

#include <QMap>
#include <QString>
#include <QVector>
#include <QtTypes>

#include "Qt-LogViewer/Models/LogEntry.h"

/**
 * @file LogPageResult.h
 * @brief Defines a loaded page together with aggregates produced by the same
 * query scan.
 */

/**
 * @struct LogPageResult
 * @brief Combines visible entries with the total result size and optional level
 * counts.
 */
struct LogPageResult {
        /** @brief Entries materialized for the requested page. */
        QVector<LogEntry> entries;
        /** @brief Total entries matching the complete query. */
        qsizetype total_entries{0};
        /** @brief Matching entries grouped by normalized level when available. */
        QMap<QString, qsizetype> log_level_counts;
        /** @brief Whether log_level_counts contains a complete level facet. */
        bool has_log_level_counts{false};
};
