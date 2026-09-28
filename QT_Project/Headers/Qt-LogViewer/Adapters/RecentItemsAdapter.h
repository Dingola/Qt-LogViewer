#pragma once

#include <QObject>
#include <QVector>

#include "Qt-LogViewer/Models/RecentItemsModel.h"
#include "Qt-LogViewer/Models/SessionTypes.h"

class SessionManager;

/**
 * @file RecentItemsAdapter.h
 * @brief Declares the adapter between session metadata and recent-item view models.
 */

/**
 * @class RecentItemsAdapter
 * @brief Keeps recent-file and recent-session models synchronized with SessionManager.
 *
 * The adapter owns both presentation models but does not own the injected session manager.
 */
class RecentItemsAdapter final: public QObject
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs the adapter and initializes both models from current session data.
         * @param session_manager Source of recent-file and recent-session records.
         * @param parent Optional QObject parent.
         */
        explicit RecentItemsAdapter(SessionManager* session_manager, QObject* parent = nullptr);

        /**
         * @brief Returns the synchronized recent-files model.
         * @return Model owned by this adapter.
         */
        [[nodiscard]] auto get_recent_files_model() -> RecentItemsModel*;

        /**
         * @brief Returns the synchronized recent-sessions model.
         * @return Model owned by this adapter.
         */
        [[nodiscard]] auto get_recent_sessions_model() -> RecentItemsModel*;

    signals:
        /**
         * @brief Emitted after either recent-items model has been updated.
         */
        auto recent_items_changed() -> void;

    private:
        /**
         * @brief Replaces recent-file model rows with mapped session records.
         * @param records Recent-file records in display order.
         */
        auto update_recent_files(const QVector<RecentLogFileRecord>& records) -> void;

        /**
         * @brief Replaces recent-session model rows with mapped session records.
         * @param records Recent-session records in display order.
         */
        auto update_recent_sessions(const QVector<RecentSessionRecord>& records) -> void;

    private:
        SessionManager* m_session_manager{nullptr};
        RecentItemsModel m_recent_files_model;
        RecentItemsModel m_recent_sessions_model;
};
