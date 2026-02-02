#pragma once

#include <Qt>

/**
 * @file RecentRoles.h
 * @brief Defines strongly-typed roles for recent items domains (files and sessions) and provides
 *        a utility to convert them to stable Qt role ids.
 *
 * The purpose is to avoid a catch-all generic namespace and keep roles domain-specific:
 *  - RecentFileRole: roles used by "Recent Files" lists.
 *  - RecentSessionRole: roles used by "Recent Sessions" lists.
 *
 * The mapping to int uses stable offsets under Qt::UserRole to preserve compatibility across views
 * and delegates.
 */

/**
 * @enum RecentFileRole
 * @brief Stable roles for recent file items.
 */
enum class RecentFileRole
{
    FilePath,   ///< Absolute file path (QString).
    FileName,   ///< File name derived from path (QString).
    AppName,    ///< Application name associated to the file (QString).
    LastOpened  ///< Last-opened timestamp (QDateTime).
};

/**
 * @enum RecentSessionRole
 * @brief Stable roles for recent session items.
 */
enum class RecentSessionRole
{
    Name,        ///< Human-readable session name (QString).
    LastOpened,  ///< Last-opened timestamp; falls back to created_at if missing (QDateTime).
    Id           ///< Unique session identifier (QString).
};

/**
 * @brief Converts a file-domain role to a stable integer role id.
 * @param role The RecentFileRole value.
 * @return Stable role id based on Qt::UserRole + offset.
 */
[[nodiscard]] constexpr auto to_role_id(RecentFileRole role) -> int
{
    // Reserve a contiguous block for file roles
    switch (role)
    {
    case RecentFileRole::FilePath:
        return Qt::UserRole + 101;
    case RecentFileRole::FileName:
        return Qt::UserRole + 102;
    case RecentFileRole::AppName:
        return Qt::UserRole + 103;
    case RecentFileRole::LastOpened:
        return Qt::UserRole + 104;
    }
    return Qt::UserRole + 199;  // fallback, should never be reached
}

/**
 * @brief Converts a session-domain role to a stable integer role id.
 * @param role The RecentSessionRole value.
 * @return Stable role id based on Qt::UserRole + offset.
 */
[[nodiscard]] constexpr auto to_role_id(RecentSessionRole role) -> int
{
    // Reserve a contiguous block for session roles
    switch (role)
    {
    case RecentSessionRole::Name:
        return Qt::UserRole + 201;
    case RecentSessionRole::LastOpened:
        return Qt::UserRole + 202;
    case RecentSessionRole::Id:
        return Qt::UserRole + 203;
    }
    return Qt::UserRole + 299;  // fallback, should never be reached
}
