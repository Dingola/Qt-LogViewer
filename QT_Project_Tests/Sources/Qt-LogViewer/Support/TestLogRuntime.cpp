/**
 * @file TestLogRuntime.cpp
 * @brief Implements the test-only composition root for log-view components.
 */

#include "Qt-LogViewer/Support/TestLogRuntime.h"

#include <QObject>

/**
 * @brief Constructs a complete isolated runtime graph.
 * @param profile Default parsing profile shared by import and tailing.
 */
TestLogRuntime::TestLogRuntime(const LogParsingProfile& profile)
    : m_profile(profile),
      m_cache_catalog(m_cache_directory.filePath(QStringLiteral("cache"))),
      m_ingest(m_profile),
      m_catalog(&m_ingest),
      m_filters(&m_views),
      m_history(m_cache_directory.filePath(QStringLiteral("history.sqlite"))),
      m_history_writer(m_history.get_database_path()),
      m_pages(&m_history, &m_views),
      m_queries(&m_filters, &m_pages, &m_views),
      m_live_tailing(m_profile, &m_history, &m_views, &m_queries),
      m_imports(m_profile, &m_ingest, &m_views, &m_history, &m_history_writer, &m_pages, &m_queries,
                &m_live_tailing, &m_cache_catalog),
      m_lifecycle(&m_views, &m_filters, &m_catalog, &m_imports, &m_live_tailing, &m_history,
                  &m_history_writer, &m_pages, &m_queries),
      m_preview(m_profile)
{
    QObject::connect(&m_imports, &LogImportCoordinator::file_removal_requested, &m_lifecycle,
                     [this](const QUuid& view_id, const QString& file_path) {
                         m_lifecycle.remove_file(view_id, file_path);
                     });
    QObject::connect(&m_imports, &LogImportCoordinator::view_removal_requested, &m_lifecycle,
                     [this](const QUuid& view_id) { m_lifecycle.close_view(view_id); });
}

/** @brief Stops asynchronous services before their collaborators are destroyed. */
TestLogRuntime::~TestLogRuntime()
{
    m_imports.shutdown();
    m_live_tailing.shutdown();
}

/** @return File catalog component. */
auto TestLogRuntime::catalog() -> FileCatalogController&
{
    return m_catalog;
}

/** @return Filter-state component. */
auto TestLogRuntime::filters() -> FilterCoordinator&
{
    return m_filters;
}

/** @return Persistent history component. */
auto TestLogRuntime::history() -> LogHistoryService&
{
    return m_history;
}

/** @return Persistent cache-generation catalog. */
auto TestLogRuntime::cache_catalog() -> LogCacheCatalog&
{
    return m_cache_catalog;
}

/** @return Import workflow component. */
auto TestLogRuntime::imports() -> LogImportCoordinator&
{
    return m_imports;
}

/** @return Live-tailing component. */
auto TestLogRuntime::live_tailing() -> LiveTailingCoordinator&
{
    return m_live_tailing;
}

/** @return Page-state component. */
auto TestLogRuntime::pages() -> LogPageCoordinator&
{
    return m_pages;
}

/** @return Preview service. */
auto TestLogRuntime::preview() -> LogPreviewService&
{
    return m_preview;
}

/** @return Query component. */
auto TestLogRuntime::queries() -> LogQueryController&
{
    return m_queries;
}

/** @return View lifecycle component. */
auto TestLogRuntime::lifecycle() -> ViewLifecycleCoordinator&
{
    return m_lifecycle;
}

/** @return View registry component. */
auto TestLogRuntime::views() -> ViewRegistry&
{
    return m_views;
}
