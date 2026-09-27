/**
 * @file LogPreviewService.cpp
 * @brief Implements bounded, side-effect-free log-file previews.
 */

#include "Qt-LogViewer/Services/LogPreviewService.h"

/**
 * @brief Constructs the preview service.
 * @param default_profile Fallback profile retained by the underlying loader.
 */
LogPreviewService::LogPreviewService(const LogParsingProfile& default_profile)
    : m_loader(default_profile)
{}

/**
 * @brief Parses at most the requested number of non-empty records.
 * @param file_path File to inspect.
 * @param profile Parsing profile to evaluate.
 * @param maximum_record_count Maximum number of outcomes to return.
 * @return Parse outcomes in source order, including structured failures.
 */
auto LogPreviewService::preview(const QString& file_path, const LogParsingProfile& profile,
                                qsizetype maximum_record_count) const -> QVector<LogParseOutcome>
{
    QVector<LogParseOutcome> outcomes =
        m_loader.preview_log_file(file_path, profile, maximum_record_count);
    return outcomes;
}
