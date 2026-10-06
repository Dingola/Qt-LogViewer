/**
 * @file LogViewerApplication.cpp
 * @brief Implements ownership and dependency composition for the Qt-LogViewer application.
 */

#include "Qt-LogViewer/Application/LogViewerApplication.h"

#include <QIcon>
#include <QString>
#include <memory>

#include "Qt-LogViewer/Adapters/RecentItemsAdapter.h"
#include "Qt-LogViewer/Controllers/FileCatalogController.h"
#include "Qt-LogViewer/Controllers/FilterCoordinator.h"
#include "Qt-LogViewer/Controllers/LiveTailingCoordinator.h"
#include "Qt-LogViewer/Controllers/LogImportCoordinator.h"
#include "Qt-LogViewer/Controllers/LogIngestController.h"
#include "Qt-LogViewer/Controllers/LogPageCoordinator.h"
#include "Qt-LogViewer/Controllers/LogQueryController.h"
#include "Qt-LogViewer/Controllers/SessionController.h"
#include "Qt-LogViewer/Controllers/ViewLifecycleCoordinator.h"
#include "Qt-LogViewer/Controllers/ViewRegistry.h"
#include "Qt-LogViewer/Services/HistoryWriteService.h"
#include "Qt-LogViewer/Services/LogCacheCatalog.h"
#include "Qt-LogViewer/Services/LogCacheReadService.h"
#include "Qt-LogViewer/Services/LogHistoryService.h"
#include "Qt-LogViewer/Services/LogParsingProfile.h"
#include "Qt-LogViewer/Services/LogPreviewService.h"
#include "Qt-LogViewer/Services/LogViewerSettings.h"
#include "Qt-LogViewer/Services/SessionManager.h"
#include "Qt-LogViewer/Services/SessionRepository.h"
#include "Qt-LogViewer/Views/MainWindow.h"
#include "QtWidgetsCommonLib/Widgets/AppWindow.h"

namespace
{
/**
 * @brief Creates and initializes a session manager before recent-item models are constructed.
 * @param repository Persistence repository owned by the application composition.
 * @return Initialized session manager.
 */
[[nodiscard]] auto create_session_manager(SessionRepository* repository)
    -> std::unique_ptr<SessionManager>
{
    auto session_manager = std::make_unique<SessionManager>(repository);
    session_manager->initialize_from_storage();
    return session_manager;
}
}  // namespace

/**
 * @class LogViewerApplication::Implementation
 * @brief Stores application-specific collaborators in their required lifetime order.
 */
class LogViewerApplication::Implementation final
{
    public:
        /**
         * @brief Constructs and connects the complete desktop application.
         * @param settings Application settings shared with the main window.
         * @param options Application-level window settings selected at startup.
         */
        explicit Implementation(LogViewerSettings& settings,
                                const LogViewerApplicationOptions& options)
            : m_default_profile(LogParsingProfile::create_default(
                  QStringLiteral("{timestamp} {level} {message} {app_name}"),
                  QStringLiteral("Qt-LogViewer default"))),
              m_ingest(m_default_profile),
              m_catalog(&m_ingest),
              m_filters(&m_views),
              m_cache_reader(&m_cache_catalog, &m_views),
              m_history_writer(m_history.get_database_path()),
              m_pages(&m_history, &m_views),
              m_queries(&m_filters, &m_pages, &m_views),
              m_live_tailing(m_default_profile, &m_history, &m_views, &m_queries),
              m_imports(m_default_profile, &m_ingest, &m_views, &m_history, &m_history_writer,
                        &m_pages, &m_queries, &m_live_tailing, &m_cache_catalog),
              m_view_lifecycle(&m_views, &m_filters, &m_catalog, &m_imports, &m_live_tailing,
                               &m_history, &m_history_writer, &m_pages, &m_queries),
              m_preview(m_default_profile),
              m_session_manager(create_session_manager(&m_session_repository)),
              m_recent_items_adapter(m_session_manager.get()),
              m_session_controller(m_session_manager.get(), m_catalog.get_model(),
                                   m_default_profile, &m_catalog, &m_views, &m_filters, &m_history,
                                   &m_pages, &m_queries, &m_imports, &m_view_lifecycle,
                                   &m_live_tailing),
              m_main_window(new MainWindow(settings, m_catalog, m_views, m_filters, m_history,
                                           m_pages, m_queries, m_imports, m_view_lifecycle,
                                           m_live_tailing, m_preview, *m_session_manager,
                                           m_recent_items_adapter, m_session_controller)),
              m_app_window(std::make_unique<QtWidgetsCommonLib::AppWindow>(nullptr, m_main_window))
        {
            m_history.set_cache_read_service(&m_cache_reader);
            m_app_window->resize(options.initial_window_size);
            m_app_window->set_app_title(options.window_title);
            m_app_window->set_app_icon(QIcon(options.window_icon_path));
            m_app_window->set_adopt_menubar(options.adopt_menubar,
                                            QtWidgetsCommonLib::WindowTitleBar::RowPosition::Top);
        }

        /**
         * @brief Shows the outer application window.
         */
        auto show() -> void
        {
            m_app_window->show();
        }

    private:
        LogParsingProfile m_default_profile;
        LogIngestController m_ingest;
        FileCatalogController m_catalog;
        ViewRegistry m_views;
        FilterCoordinator m_filters;
        LogCacheCatalog m_cache_catalog;
        LogCacheReadService m_cache_reader;
        LogHistoryService m_history;
        HistoryWriteService m_history_writer;
        LogPageCoordinator m_pages;
        LogQueryController m_queries;
        LiveTailingCoordinator m_live_tailing;
        LogImportCoordinator m_imports;
        ViewLifecycleCoordinator m_view_lifecycle;
        LogPreviewService m_preview;
        SessionRepository m_session_repository;
        std::unique_ptr<SessionManager> m_session_manager;
        RecentItemsAdapter m_recent_items_adapter;
        SessionController m_session_controller;
        MainWindow* m_main_window{nullptr};  ///< Owned by the outer application window.
        std::unique_ptr<QtWidgetsCommonLib::AppWindow> m_app_window;
};

/**
 * @brief Constructs the Qt-LogViewer application composition.
 * @param settings Application settings that outlive this composition root.
 * @param options Application-level window settings selected at startup.
 */
LogViewerApplication::LogViewerApplication(LogViewerSettings& settings,
                                           const LogViewerApplicationOptions& options)
    : m_implementation(std::make_unique<Implementation>(settings, options))
{}

/**
 * @brief Destroys the composed application in reverse dependency order.
 */
LogViewerApplication::~LogViewerApplication() = default;

/**
 * @brief Shows the composed application window.
 */
auto LogViewerApplication::show() -> void
{
    m_implementation->show();
}
