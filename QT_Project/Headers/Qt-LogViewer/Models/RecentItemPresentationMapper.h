#pragma once

#include "Qt-LogViewer/Models/SessionTypes.h"
#include "QtWidgetsCommonLib/RecentItems/RecentItemPresentation.h"

namespace QtCommonLib
{
struct LocalFileStatus;
}

/**
 * @file RecentItemPresentationMapper.h
 * @brief Declares Qt-LogViewer mappings from persisted recent records to
 * presentation items.
 */

/**
 * @namespace RecentItemPresentationMapper
 * @brief Maps Qt-LogViewer domain records onto the generic recent-item
 * presentation contract.
 */
namespace RecentItemPresentationMapper
{
/**
 * @brief Maps a recent log-file record and its transient file status.
 * @param record Persisted recent log-file record.
 * @param file_status Current local filesystem status.
 * @return Presentation values for the recent log file.
 */
[[nodiscard]] auto from_recent_log_file(const RecentLogFileRecord& record,
                                        const QtCommonLib::LocalFileStatus& file_status)
    -> QtWidgetsCommonLib::RecentItemPresentation;

/**
 * @brief Maps a recent session record.
 * @param record Persisted recent session record.
 * @return Presentation values for the recent session.
 */
[[nodiscard]] auto from_recent_session(const RecentSessionRecord& record)
    -> QtWidgetsCommonLib::RecentItemPresentation;
}  // namespace RecentItemPresentationMapper
