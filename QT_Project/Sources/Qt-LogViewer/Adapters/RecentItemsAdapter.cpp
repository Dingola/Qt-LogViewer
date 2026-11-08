/**
 * @file RecentItemsAdapter.cpp
 * @brief Implements synchronization of recent session data with presentation models.
 */

#include "Qt-LogViewer/Adapters/RecentItemsAdapter.h"

#include <QCoreApplication>
#include <QGuiApplication>
#include <QStringList>
#include <QVector>

#include "Qt-LogViewer/Models/RecentItemPresentationMapper.h"
#include "Qt-LogViewer/Services/RecentFilesService.h"
#include "Qt-LogViewer/Services/SessionManager.h"
#include "QtWidgetsCommonLib/RecentItems/RecentFilesMenuController.h"

namespace
{
using QtWidgetsCommonLib::RecentItemPresentation;
using QtWidgetsCommonLib::RecentItemsModel;
}  // namespace

/**
 * @brief Constructs the adapter and binds its models to SessionManager updates.
 * @param recent_files_service Source of recent-file records.
 * @param session_manager Source of recent-session records.
 * @param parent Optional QObject parent.
 */
RecentItemsAdapter::RecentItemsAdapter(RecentFilesService* recent_files_service,
                                       SessionManager* session_manager, QObject* parent)
    : QObject(parent),
      m_recent_files_service(recent_files_service),
      m_session_manager(session_manager)
{
    update_recent_sessions(m_session_manager->get_recent_sessions());

    connect(m_recent_files_service, &RecentFilesService::recent_log_files_changed, this,
            [this](const QVector<RecentLogFileRecord>& records) { update_recent_files(records); });
    connect(
        m_session_manager, &SessionManager::recent_sessions_changed, this,
        [this](const QVector<RecentSessionRecord>& records) { update_recent_sessions(records); });

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
 * @brief Attaches the high-level recent-files facade and supplies current persisted paths.
 * @param controller Facade owned by the menu controller.
 */
auto RecentItemsAdapter::bind_recent_files_controller(
    QtWidgetsCommonLib::RecentFilesMenuController* controller) -> void
{
    m_recent_files_controller = controller;
    if (m_recent_files_controller != nullptr)
    {
        connect(m_recent_files_controller, &QObject::destroyed, this,
                [this] { m_recent_files_controller = nullptr; });
        update_recent_files(m_recent_files_service->get_recent_log_files());
    }
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
    if (m_recent_files_controller != nullptr)
    {
        m_recent_files_controller->refresh_file_status();
    }
}

/**
 * @brief Replaces recent-file presentation items with mapped records.
 * @param records Recent-file records in display order.
 */
auto RecentItemsAdapter::update_recent_files(const QVector<RecentLogFileRecord>& records) -> void
{
    if (m_recent_files_controller != nullptr)
    {
        QStringList file_paths;
        file_paths.reserve(records.size());
        for (const RecentLogFileRecord& record: records)
        {
            file_paths.push_back(record.file_path);
        }
        m_recent_files_controller->set_files(std::move(file_paths));
    }
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
