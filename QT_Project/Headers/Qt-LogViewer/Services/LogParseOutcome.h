#pragma once

#include <QString>
#include <optional>

#include "Qt-LogViewer/Models/LogEntry.h"
#include "QtRecordParser/ParseResult.h"

/**
 * @file LogParseOutcome.h
 * @brief Defines the explicit result of adapting one raw log record.
 */

/**
 * @struct LogParseOutcome
 * @brief Retains parser success or failure together with source context and the adapted entry.
 *
 * The generic parse result remains available even after successful LogEntry adaptation so that
 * callers can inspect custom fields. A failed parse has no adapted entry and retains the
 * structured QtRecordParser error instead.
 */
struct LogParseOutcome {
        /** Generic record values or structured parser failure information. */
        QtRecordParser::ParseResult parse_result;

        /** Adapted log entry; empty when parsing or conversion failed. */
        std::optional<LogEntry> entry;

        /** Unmodified input record supplied to LogParser::parse_line(). */
        QString raw_record;

        /** One-based source line number, or -1 when the caller cannot determine it. */
        qsizetype line_number{-1};

        /**
         * @brief Returns whether parsing and LogEntry adaptation both succeeded.
         * @return True when parse_result is successful and entry contains a value.
         */
        [[nodiscard]] auto succeeded() const noexcept -> bool
        {
            return parse_result.succeeded() && entry.has_value();
        }
};
