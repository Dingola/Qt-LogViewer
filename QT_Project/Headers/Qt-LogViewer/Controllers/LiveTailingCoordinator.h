#pragma once

#include <QObject>
#include <QSet>
#include <QUuid>

#include "Qt-LogViewer/Models/LogEntry.h"
#include "Qt-LogViewer/Services/LogParsingProfile.h"

class LogHistoryService;
class LogQueryController;
class LogTailerService;
class QTimer;
class ViewRegistry;

/**
 * @file LiveTailingCoordinator.h
 * @brief Declares coordination of live-tail state, persistence, and visible updates.
 */

/**
 * @class LiveTailingCoordinator
 * @brief Owns the complete live-tailing lifecycle for log views.
 *
 * The coordinator keeps enabled-view state, manages low-level file registrations, persists
 * appended entries, and batches query updates. LogTailerService remains responsible only for
 * reading and parsing appended records.
 */
class LiveTailingCoordinator final: public QObject
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs a live-tailing coordinator.
         * @param default_profile Fallback profile for files without a retained profile.
         * @param history_service Storage receiving appended entries.
         * @param views Registry containing live view state and file profiles.
         * @param queries Query component used for visible page updates.
         * @param parent Optional QObject parent.
         */
        explicit LiveTailingCoordinator(const LogParsingProfile& default_profile,
                                        LogHistoryService* history_service, ViewRegistry* views,
                                        LogQueryController* queries, QObject* parent = nullptr);

        /**
         * @brief Stops all registrations before owned objects are destroyed.
         */
        ~LiveTailingCoordinator() override;

        /**
         * @brief Enables or disables live tailing for a view.
         *
         * Enabling registers every currently loaded file with its retained parsing profile.
         * Disabling removes registrations and pending visible updates for the view.
         *
         * @param view_id Target view.
         * @param enabled Whether live tailing should be active.
         */
        auto set_enabled(const QUuid& view_id, bool enabled) -> void;

        /**
         * @brief Returns whether live tailing is enabled for a view.
         * @param view_id Target view.
         * @return True when the view is enabled.
         */
        [[nodiscard]] auto is_enabled(const QUuid& view_id) const -> bool;

        /**
         * @brief Resets registrations while retaining a requested restore-time setting.
         *
         * No file is started immediately. Restored imports call start_file() only after their
         * history writes have completed successfully.
         *
         * @param view_id Restored view.
         * @param enabled Desired live-tailing state.
         */
        auto reset_view(const QUuid& view_id, bool enabled) -> void;

        /**
         * @brief Starts one file when its view is enabled.
         * @param view_id Owning view.
         * @param file_path Imported file path.
         * @param profile Profile selected for the file.
         */
        auto start_file(const QUuid& view_id, const QString& file_path,
                        const LogParsingProfile& profile) -> void;

        /**
         * @brief Stops one file registration and drops no view-level setting.
         * @param view_id Owning view.
         * @param file_path Removed file path.
         */
        auto stop_file(const QUuid& view_id, const QString& file_path) -> void;

        /**
         * @brief Removes all live-tail state belonging to a closing view.
         * @param view_id Closing view.
         */
        auto remove_view(const QUuid& view_id) -> void;

        /**
         * @brief Stops timers and every low-level file registration.
         */
        auto shutdown() -> void;

        /**
         * @brief Sets the interval used to combine visible updates.
         * @param interval_ms Non-negative batching interval in milliseconds.
         */
        auto set_refresh_interval_ms(int interval_ms) -> void;

    private:
        /**
         * @brief Persists appended entries and schedules one visible update for their view.
         * @param view_id Receiving view.
         * @param entries Newly parsed entries.
         */
        auto handle_entries(const QUuid& view_id, const QVector<LogEntry>& entries) -> void;

        /**
         * @brief Applies the accumulated visible updates through the query component.
         */
        auto refresh_pending_views() -> void;

    private:
        LogParsingProfile m_default_profile;
        LogHistoryService* m_history_service{nullptr};
        ViewRegistry* m_views{nullptr};
        LogQueryController* m_queries{nullptr};
        LogTailerService* m_tailer_service{nullptr};
        QTimer* m_refresh_timer{nullptr};
        QSet<QUuid> m_enabled_views;
        QSet<QUuid> m_pending_refresh_views;
        bool m_shutting_down{false};
};
