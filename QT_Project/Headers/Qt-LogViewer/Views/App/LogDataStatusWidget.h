#pragma once

#include <QWidget>

class QLabel;

/**
 * @file LogDataStatusWidget.h
 * @brief Declares the compact active-view data status presentation.
 */

/**
 * @enum LogCacheStatus
 * @brief Describes whether the active view can use a current persistent cache.
 */
enum class LogCacheStatus
{
    Unavailable,
    Current,
    Updating,
};

/**
 * @class LogDataStatusWidget
 * @brief Presents compact entry, source, filter, and index information in the status bar.
 */
class LogDataStatusWidget final: public QWidget
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs an initially hidden data status widget.
         * @param parent Optional parent widget.
         */
        explicit LogDataStatusWidget(QWidget* parent = nullptr);

        /**
         * @brief Updates all active-view status values and shows the widget.
         * @param visible_entries Entries matching the active filters.
         * @param total_entries All entries belonging to the active view.
         * @param source_count Number of loaded sources.
         * @param active_filter_count Number of active filter categories.
         * @param cache_status Current persistent-cache state represented as an index status.
         */
        auto set_status(qsizetype visible_entries, qsizetype total_entries, int source_count,
                        int active_filter_count, LogCacheStatus cache_status) -> void;

        /** @brief Clears and hides the status when no log view is active. */
        auto clear_status() -> void;

    private:
        /**
         * @brief Updates the user-facing index label and its style property.
         * @param cache_status Cache state to present.
         */
        auto set_cache_status(LogCacheStatus cache_status) -> void;

        QLabel* m_entries_label{nullptr};
        QLabel* m_sources_label{nullptr};
        QLabel* m_filters_label{nullptr};
        QLabel* m_cache_separator{nullptr};
        QLabel* m_cache_label{nullptr};
};
