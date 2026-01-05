#pragma once

#include <QMap>
#include <QObject>
#include <QSet>
#include <QString>
#include <QUuid>
#include <QVector>

#include "Qt-LogViewer/Controllers/ViewRegistry.h"
#include "Qt-LogViewer/Models/LogEntry.h"
#include "Qt-LogViewer/Models/LogSortFilterProxyModel.h"

/**
 * @file FilterCoordinator.h
 * @brief Coordinates per-view filtering and file visibility using `ViewRegistry` and proxies.
 *
 * Responsibilities:
 * - Delegate application-name, log-level, and search filters to a view's `LogSortFilterProxyModel`.
 * - Coordinate file-level visibility controls (show-only, toggle, hide).
 * - Provide query helpers for current filters and search parameters.
 * - Compute per-view log level counts based on current entries from `ViewRegistry`.
 * - Expose static available log levels identical across views.
 * - Adjust per-view visibility state when files are removed (show-only reset, hidden set updates).
 */
class FilterCoordinator: public QObject
{
        Q_OBJECT

    public:
        /**
         * @brief Construct a new FilterCoordinator.
         * @param views Non-owning pointer to the `ViewRegistry` for context/proxy lookups.
         * @param parent Optional QObject parent.
         */
        explicit FilterCoordinator(ViewRegistry* views, QObject* parent = nullptr);

        /**
         * @brief Set application name filter for a specific view.
         * @param view_id Target view.
         * @param app_name Application name to filter by.
         */
        auto set_app_name(const QUuid& view_id, const QString& app_name) -> void;

        /**
         * @brief Set log level filters for a specific view.
         * @param view_id Target view.
         * @param levels Set of log level names (e.g., "Info", "Error").
         */
        auto set_log_levels(const QUuid& view_id, const QSet<QString>& levels) -> void;

        /**
         * @brief Set search filter for a specific view.
         * @param view_id Target view.
         * @param text Search text or regex.
         * @param field Field to search in.
         * @param use_regex Whether to treat text as regex.
         */
        auto set_search(const QUuid& view_id, const QString& text, const QString& field,
                        bool use_regex) -> void;

        /**
         * @brief Apply a "show only file" visibility filter for the specified view.
         *        Pass empty string to reset (show all).
         * @param view_id Target view.
         * @param file_path File path to show exclusively, or empty to reset.
         */
        auto set_show_only(const QUuid& view_id, const QString& file_path) -> void;

        /**
         * @brief Toggle visibility of a file in the specified view (hide/unhide).
         *        Honors current "show only" rules.
         * @param view_id Target view.
         * @param file_path Absolute file path to toggle.
         */
        auto toggle_visibility(const QUuid& view_id, const QString& file_path) -> void;

        /**
         * @brief Hide a specific file in the specified view.
         * @param view_id Target view.
         * @param file_path File path to hide.
         */
        auto hide_file(const QUuid& view_id, const QString& file_path) -> void;

        /**
         * @brief Get current application name filter for a view.
         * @param view_id Target view.
         * @return Application name filter string (empty if none).
         */
        [[nodiscard]] auto get_app_name(const QUuid& view_id) const -> QString;

        /**
         * @brief Get current log level filters for a view.
         * @param view_id Target view.
         * @return Set of log level names.
         */
        [[nodiscard]] auto get_log_levels(const QUuid& view_id) const -> QSet<QString>;

        /**
         * @brief Get current search text for a view.
         * @param view_id Target view.
         * @return Search text string.
         */
        [[nodiscard]] auto get_search_text(const QUuid& view_id) const -> QString;

        /**
         * @brief Get current search field for a view.
         * @param view_id Target view.
         * @return Search field string.
         */
        [[nodiscard]] auto get_search_field(const QUuid& view_id) const -> QString;

        /**
         * @brief Get whether the current search uses regex for a view.
         * @param view_id Target view.
         * @return True if regex, false otherwise.
         */
        [[nodiscard]] auto is_search_regex(const QUuid& view_id) const -> bool;

        /**
         * @brief Compute per-view log level counts using entries from `ViewRegistry`.
         * @param view_id Target view.
         * @return Map of level name -> count.
         */
        [[nodiscard]] auto get_log_level_counts(const QUuid& view_id) const -> QMap<QString, int>;

        /**
         * @brief Static list of available log levels (same across all views).
         * @return Vector of log level names.
         */
        [[nodiscard]] static auto get_available_log_levels() -> QVector<QString>;

        /**
         * @brief Adjust visibility state for a view when a file is removed.
         *
         * Behavior:
         * - If the removed file was the current show-only target, clear show-only and hide all
         * remaining files (empty view).
         * - Otherwise, remove the file from the hidden set if present.
         *
         * @param view_id Target view id.
         * @param file_path Removed absolute file path.
         */
        auto adjust_visibility_on_file_removed(const QUuid& view_id,
                                               const QString& file_path) -> void;

        /**
         * @brief Adjust visibility across all views when a file is removed globally.
         *
         * For each view applies the same rules as `adjust_visibility_on_file_removed`.
         * @param file_path Removed absolute file path.
         */
        auto adjust_visibility_on_global_file_removed(const QString& file_path) -> void;

    private:
        /**
         * @brief Helper to access the sort/filter proxy for a given view.
         * @param view_id Target view.
         * @return Proxy pointer or nullptr.
         */
        [[nodiscard]] auto get_sort_filter_proxy(const QUuid& view_id) const
            -> LogSortFilterProxyModel*;

        ViewRegistry* m_views{nullptr};
};
