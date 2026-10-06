#pragma once

#include <QTemporaryDir>

#include "Qt-LogViewer/Controllers/FileCatalogController.h"
#include "Qt-LogViewer/Controllers/FilterCoordinator.h"
#include "Qt-LogViewer/Controllers/LiveTailingCoordinator.h"
#include "Qt-LogViewer/Controllers/LogImportCoordinator.h"
#include "Qt-LogViewer/Controllers/LogIngestController.h"
#include "Qt-LogViewer/Controllers/LogPageCoordinator.h"
#include "Qt-LogViewer/Controllers/LogQueryController.h"
#include "Qt-LogViewer/Controllers/ViewLifecycleCoordinator.h"
#include "Qt-LogViewer/Controllers/ViewRegistry.h"
#include "Qt-LogViewer/Services/HistoryWriteService.h"
#include "Qt-LogViewer/Services/LogCacheCatalog.h"
#include "Qt-LogViewer/Services/LogHistoryService.h"
#include "Qt-LogViewer/Services/LogParsingProfile.h"
#include "Qt-LogViewer/Services/LogPreviewService.h"

/**
 * @file TestLogRuntime.h
 * @brief Declares a test-only composition root for focused log-view components.
 */

/**
 * @class TestLogRuntime
 * @brief Owns the production component graph used by integration-style tests.
 *
 * The helper contains no forwarding application API. Tests access the focused components
 * directly, keeping their dependencies visible after removal of the legacy facade.
 */
class TestLogRuntime final
{
    public:
        /**
         * @brief Constructs a complete isolated runtime graph.
         * @param profile Default parsing profile shared by import and tailing.
         */
        explicit TestLogRuntime(const LogParsingProfile& profile);

        /** @brief Stops asynchronous services before their collaborators are destroyed. */
        ~TestLogRuntime();

        TestLogRuntime(const TestLogRuntime&) = delete;
        auto operator=(const TestLogRuntime&) -> TestLogRuntime& = delete;

        /** @return File catalog component. */
        [[nodiscard]] auto catalog() -> FileCatalogController&;

        /** @return Filter-state component. */
        [[nodiscard]] auto filters() -> FilterCoordinator&;

        /** @return Persistent history component. */
        [[nodiscard]] auto history() -> LogHistoryService&;

        /** @return Persistent cache-generation catalog. */
        [[nodiscard]] auto cache_catalog() -> LogCacheCatalog&;

        /** @return Import workflow component. */
        [[nodiscard]] auto imports() -> LogImportCoordinator&;

        /** @return Live-tailing component. */
        [[nodiscard]] auto live_tailing() -> LiveTailingCoordinator&;

        /** @return Page-state component. */
        [[nodiscard]] auto pages() -> LogPageCoordinator&;

        /** @return Preview service. */
        [[nodiscard]] auto preview() -> LogPreviewService&;

        /** @return Query component. */
        [[nodiscard]] auto queries() -> LogQueryController&;

        /** @return View lifecycle component. */
        [[nodiscard]] auto lifecycle() -> ViewLifecycleCoordinator&;

        /** @return View registry component. */
        [[nodiscard]] auto views() -> ViewRegistry&;

    private:
        LogParsingProfile m_profile;
        QTemporaryDir m_cache_directory;
        LogCacheCatalog m_cache_catalog;
        LogIngestController m_ingest;
        FileCatalogController m_catalog;
        ViewRegistry m_views;
        FilterCoordinator m_filters;
        LogHistoryService m_history;
        HistoryWriteService m_history_writer;
        LogPageCoordinator m_pages;
        LogQueryController m_queries;
        LiveTailingCoordinator m_live_tailing;
        LogImportCoordinator m_imports;
        ViewLifecycleCoordinator m_lifecycle;
        LogPreviewService m_preview;
};
