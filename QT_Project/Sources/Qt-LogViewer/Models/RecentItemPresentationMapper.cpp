/**
 * @file RecentItemPresentationMapper.cpp
 * @brief Implements Qt-LogViewer recent-record presentation mappings.
 */

#include "Qt-LogViewer/Models/RecentItemPresentationMapper.h"

using QtWidgetsCommonLib::RecentItemPresentation;
/**
 * @brief Maps a recent session record.
 * @param record Persisted recent session record.
 * @return Presentation values for the recent session.
 */
auto RecentItemPresentationMapper::from_recent_session(const RecentSessionRecord& record)
    -> RecentItemPresentation
{
    RecentItemPresentation result;
    result.identity = record.id;
    result.title = record.name;
    result.secondary_text = record.id;
    result.last_used = record.last_opened.isValid() ? record.last_opened : record.created_at;
    return result;
}
