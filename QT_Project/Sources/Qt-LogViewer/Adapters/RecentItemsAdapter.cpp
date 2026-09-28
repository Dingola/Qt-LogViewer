/**
 * @file RecentItemsAdapter.cpp
 * @brief Implements synchronization of recent session data with presentation models.
 */

#include "Qt-LogViewer/Adapters/RecentItemsAdapter.h"

#include <QHash>
#include <QVariant>
#include <QVector>

#include "Qt-LogViewer/Models/RecentListSchema.h"
#include "Qt-LogViewer/Services/SessionManager.h"

/**
 * @brief Constructs the adapter and binds its models to SessionManager updates.
 * @param session_manager Source of recent-file and recent-session records.
 * @param parent Optional QObject parent.
 */
RecentItemsAdapter::RecentItemsAdapter(SessionManager* session_manager, QObject* parent)
    : QObject(parent),
      m_session_manager(session_manager),
      m_recent_files_model(RecentListSchemas::make_recent_files_schema()),
      m_recent_sessions_model(RecentListSchemas::make_recent_sessions_schema())
{
    update_recent_files(m_session_manager->get_recent_log_files());
    update_recent_sessions(m_session_manager->get_recent_sessions());

    connect(m_session_manager, &SessionManager::recent_log_files_changed, this,
            [this](const QVector<RecentLogFileRecord>& records) {
                update_recent_files(records);
                emit recent_items_changed();
            });
    connect(m_session_manager, &SessionManager::recent_sessions_changed, this,
            [this](const QVector<RecentSessionRecord>& records) {
                update_recent_sessions(records);
                emit recent_items_changed();
            });
}

/**
 * @brief Returns the synchronized recent-files model.
 * @return Model owned by this adapter.
 */
auto RecentItemsAdapter::get_recent_files_model() -> RecentItemsModel*
{
    RecentItemsModel* model = &m_recent_files_model;
    return model;
}

/**
 * @brief Returns the synchronized recent-sessions model.
 * @return Model owned by this adapter.
 */
auto RecentItemsAdapter::get_recent_sessions_model() -> RecentItemsModel*
{
    RecentItemsModel* model = &m_recent_sessions_model;
    return model;
}

/**
 * @brief Replaces recent-file model rows with mapped session records.
 * @param records Recent-file records in display order.
 */
auto RecentItemsAdapter::update_recent_files(const QVector<RecentLogFileRecord>& records) -> void
{
    QVector<QHash<int, QVariant>> rows;
    rows.reserve(records.size());

    for (const RecentLogFileRecord& record: records)
    {
        rows.push_back(RecentListSchemas::build_recent_file_row(record));
    }

    m_recent_files_model.set_rows(std::move(rows));
}

/**
 * @brief Replaces recent-session model rows with mapped session records.
 * @param records Recent-session records in display order.
 */
auto RecentItemsAdapter::update_recent_sessions(const QVector<RecentSessionRecord>& records) -> void
{
    QVector<QHash<int, QVariant>> rows;
    rows.reserve(records.size());

    for (const RecentSessionRecord& record: records)
    {
        rows.push_back(RecentListSchemas::build_recent_session_row(record));
    }

    m_recent_sessions_model.set_rows(std::move(rows));
}
