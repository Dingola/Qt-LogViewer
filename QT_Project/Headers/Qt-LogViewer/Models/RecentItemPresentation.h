#pragma once

#include <QDateTime>
#include <QIcon>
#include <QString>
#include <QVariant>

/**
 * @file RecentItemPresentation.h
 * @brief Defines the application-independent presentation contract for a recent
 * item.
 */

/**
 * @struct RecentItemPresentation
 * @brief Contains the semantic values required to present and activate a recent
 * item.
 *
 * Domain records remain responsible for persistence. This type contains only
 * values needed by views, delegates, and menus and can therefore represent
 * files, sessions, or other recent items.
 */
struct RecentItemPresentation {
        QVariant identity;       ///< Stable application-owned identity used for activation.
        QString title;           ///< Primary user-facing text.
        QString secondary_text;  ///< Supporting user-facing text.
        QDateTime last_used;     ///< Most recent usage timestamp.
        QIcon icon;              ///< Optional presentation icon.
        bool enabled{true};      ///< Whether the item can currently be activated.
        QString status_text;     ///< Optional short status description.
        QString tooltip;         ///< Optional detailed explanatory text.
};
