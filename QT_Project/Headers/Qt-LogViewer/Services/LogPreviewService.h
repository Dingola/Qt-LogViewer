#pragma once

#include <QString>
#include <QVector>

#include "Qt-LogViewer/Services/LogLoader.h"
#include "Qt-LogViewer/Services/LogParseOutcome.h"
#include "Qt-LogViewer/Services/LogParsingProfile.h"

/**
 * @file LogPreviewService.h
 * @brief Declares bounded, read-only log-file previews.
 */

/**
 * @class LogPreviewService
 * @brief Parses a limited sample without registering or importing a file.
 *
 * Preview parsing is intentionally independent of the import queue. A dialog can therefore
 * evaluate parsing profiles without changing view, history, or asynchronous import state.
 */
class LogPreviewService final
{
    public:
        /**
         * @brief Constructs the preview service.
         * @param default_profile Fallback profile retained by the underlying loader.
         */
        explicit LogPreviewService(const LogParsingProfile& default_profile);

        /**
         * @brief Parses at most the requested number of non-empty records.
         * @param file_path File to inspect.
         * @param profile Parsing profile to evaluate.
         * @param maximum_record_count Maximum number of outcomes to return.
         * @return Parse outcomes in source order, including structured failures.
         */
        [[nodiscard]] auto preview(const QString& file_path, const LogParsingProfile& profile,
                                   qsizetype maximum_record_count) const
            -> QVector<LogParseOutcome>;

    private:
        LogLoader m_loader;
};
