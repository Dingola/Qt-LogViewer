#pragma once

#include <QObject>
#include <QVector>

#include "Qt-LogViewer/Models/SessionTypes.h"
#include "QtWidgetsCommonLib/RecentItems/RecentItemsModel.h"

class SessionManager;
class RecentFilesService;
namespace QtWidgetsCommonLib
{
class RecentFilesMenuController;
}

/**
 * @file RecentItemsAdapter.h
 * @brief Declares the adapter between session metadata and recent-item view models.
 */

/**
 * @class RecentItemsAdapter
 * @brief Keeps recent-file and recent-session presentation synchronized with domain services.
 *
 * The adapter owns the recent-session model and synchronizes recent-file paths with an attached
 * menu facade. It does not own the injected domain services or facade.
 */
class RecentItemsAdapter final: public QObject
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs the adapter and initializes recent-session presentation data.
         * @param recent_files_service Source of recent-file records.
         * @param session_manager Source of recent-session records.
         * @param parent Optional QObject parent.
         */
        explicit RecentItemsAdapter(RecentFilesService* recent_files_service,
                                    SessionManager* session_manager, QObject* parent = nullptr);

        /**
         * @brief Attaches the high-level recent-files facade and supplies current persisted paths.
         * @param controller Facade owned by the menu controller.
         */
        auto bind_recent_files_controller(QtWidgetsCommonLib::RecentFilesMenuController* controller)
            -> void;

        /**
         * @brief Returns the synchronized recent-sessions model.
         * @return Model owned by this adapter.
         */
        [[nodiscard]] auto get_recent_sessions_model() -> QtWidgetsCommonLib::RecentItemsModel*;

        /**
         * @brief Re-evaluates transient file availability for all persisted recent-file records.
         */
        auto refresh_file_availability() -> void;

    private:
        /**
         * @brief Replaces recent-file presentation items with mapped records.
         * @param records Recent-file records in display order.
         */
        auto update_recent_files(const QVector<RecentLogFileRecord>& records) -> void;

        /**
         * @brief Replaces recent-session presentation items with mapped records.
         * @param records Recent-session records in display order.
         */
        auto update_recent_sessions(const QVector<RecentSessionRecord>& records) -> void;

    private:
        RecentFilesService* m_recent_files_service{nullptr};
        SessionManager* m_session_manager{nullptr};
        QtWidgetsCommonLib::RecentFilesMenuController* m_recent_files_controller{nullptr};
        QtWidgetsCommonLib::RecentItemsModel m_recent_sessions_model;
};
