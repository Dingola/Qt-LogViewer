/**
 * @file RecentItemsAdapter.cpp
 * @brief Implements synchronization of recent session data with presentation models.
 */

#include "Qt-LogViewer/Adapters/RecentItemsAdapter.h"

#include <QCoreApplication>
#include <QGuiApplication>
#include <QVector>

#include "Qt-LogViewer/Models/RecentItemPresentationMapper.h"
#include "Qt-LogViewer/Services/LocalFileStatus.h"
#include "Qt-LogViewer/Services/SessionManager.h"

/**
 * @brief Constructs the adapter and binds its models to SessionManager updates.
 * @param session_manager Source of recent-file and recent-session records.
 * @param parent Optional QObject parent.
 */
RecentItemsAdapter::RecentItemsAdapter(SessionManager* session_manager, QObject* parent)
    : QObject(parent), m_session_manager(session_manager)
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

    QGuiApplication* application = qobject_cast<QGuiApplication*>(QCoreApplication::instance());
    if (application != nullptr)
    {
        connect(application, &QGuiApplication::applicationStateChanged, this,
                [this](Qt::ApplicationState state) {
                    if (state == Qt::ApplicationActive)
                    {
                        refresh_file_availability();
                    }
                });
    }
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
 * @brief Re-evaluates transient file availability for all persisted recent-file records.
 */
auto RecentItemsAdapter::refresh_file_availability() -> void
{
    update_recent_files(m_session_manager->get_recent_log_files());
    emit recent_items_changed();
}

/**
 * @brief Replaces recent-file presentation items with mapped records.
 * @param records Recent-file records in display order.
 */
auto RecentItemsAdapter::update_recent_files(const QVector<RecentLogFileRecord>& records) -> void
{
    QVector<RecentItemPresentation> items;
    items.reserve(records.size());

    for (const RecentLogFileRecord& record: records)
    {
        const QtCommonLib::LocalFileStatus file_status =
            QtCommonLib::inspect_local_file(record.file_path);
        items.push_back(RecentItemPresentationMapper::from_recent_log_file(record, file_status));
    }

    m_recent_files_model.set_items(std::move(items));
}

/**
 * @brief Replaces recent-session presentation items with mapped records.
 * @param records Recent-session records in display order.
 */
auto RecentItemsAdapter::update_recent_sessions(const QVector<RecentSessionRecord>& records) -> void
{
    QVector<RecentItemPresentation> items;
    items.reserve(records.size());

    for (const RecentSessionRecord& record: records)
    {
        items.push_back(RecentItemPresentationMapper::from_recent_session(record));
    }

    m_recent_sessions_model.set_items(std::move(items));
}
