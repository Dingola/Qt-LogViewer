#pragma once

#include "Qt-LogViewer/Models/SessionTypes.h"
#include "QtWidgetsCommonLib/RecentItems/RecentItemPresentation.h"

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
 * @brief Maps a recent session record.
 * @param record Persisted recent session record.
 * @return Presentation values for the recent session.
 */
[[nodiscard]] auto from_recent_session(const RecentSessionRecord& record)
    -> QtWidgetsCommonLib::RecentItemPresentation;
}  // namespace RecentItemPresentationMapper
