#pragma once

#include <Qt>

/**
 * @file RecentRoles.h
 * @brief Defines stable semantic roles shared by all recent-item presentations.
 */

/**
 * @enum RecentItemRole
 * @brief Identifies values exposed by `RecentItemsModel` independently of the item domain.
 */
enum class RecentItemRole
{
    Identity,       ///< Stable application-owned identity stored as QVariant.
    Title,          ///< Primary user-facing text stored as QString.
    SecondaryText,  ///< Supporting user-facing text stored as QString.
    LastUsed,       ///< Most recent usage timestamp stored as QDateTime.
    Icon,           ///< Optional presentation icon stored as QIcon.
    Enabled,        ///< Whether the item can currently be activated.
    StatusText,     ///< Optional short status description.
    ToolTip         ///< Optional detailed explanatory text.
};

/**
 * @brief Converts a semantic recent-item role to its stable Qt role id.
 * @param role Recent-item role to convert.
 * @return Stable role id based on `Qt::UserRole`.
 */
[[nodiscard]] constexpr auto to_role_id(RecentItemRole role) -> int
{
    int role_id = Qt::UserRole + 108;
    switch (role)
    {
    case RecentItemRole::Identity:
        role_id = Qt::UserRole + 101;
        break;
    case RecentItemRole::Title:
        role_id = Qt::UserRole + 102;
        break;
    case RecentItemRole::SecondaryText:
        role_id = Qt::UserRole + 103;
        break;
    case RecentItemRole::LastUsed:
        role_id = Qt::UserRole + 104;
        break;
    case RecentItemRole::Icon:
        role_id = Qt::UserRole + 105;
        break;
    case RecentItemRole::Enabled:
        role_id = Qt::UserRole + 106;
        break;
    case RecentItemRole::StatusText:
        role_id = Qt::UserRole + 107;
        break;
    case RecentItemRole::ToolTip:
        role_id = Qt::UserRole + 108;
        break;
    }
    return role_id;
}
