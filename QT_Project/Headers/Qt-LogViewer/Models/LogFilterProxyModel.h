#pragma once

#include <QRegularExpression>
#include <QSet>
#include <QSortFilterProxyModel>
#include <QString>

/**
 * @file LogFilterProxyModel.h
 * @brief This file contains the definition of the LogFilterProxyModel class.
 */

/**
 * @class LogFilterProxyModel
 * @brief Proxy model for filtering, searching, and paging log entries in the LogModel.
 *
 * Supports filtering by application name, log level, search string (plain or regex)
 * in a selected field, and paging of results.
 */
class LogFilterProxyModel: public QSortFilterProxyModel
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs a LogFilterProxyModel object.
         * @param parent The parent QObject.
         */
        explicit LogFilterProxyModel(QObject* parent = nullptr);

        /**
         * @brief Sets the application name filter.
         * @param app_name The application name to filter by (empty for no filter).
         */
        auto set_app_name_filter(const QString& app_name) -> void;

        /**
         * @brief Sets the set of log levels to filter by.
         * @param levels The set of log levels (e.g., {"Info", "Error"}).
         */
        auto set_level_filter(const QSet<QString>& levels) -> void;

        /**
         * @brief Sets the search string and field.
         * @param search_text The text or regex to search for.
         * @param field The field to search in ("Message", "Level", "AppName", etc.).
         * @param use_regex Whether to interpret search_text as a regular expression.
         */
        auto set_search_filter(const QString& search_text, const QString& field,
                               bool use_regex) -> void;

        /**
         * @brief Enables or disables paging.
         * @param enabled True to enable paging, false to show all results.
         */
        auto set_paging_enabled(bool enabled) -> void;

        /**
         * @brief Sets the number of items per page.
         * @param size The number of items per page.
         */
        auto set_page_size(int size) -> void;

        /**
         * @brief Sets the current page (1-based).
         * @param page The page number to display.
         */
        auto set_current_page(int page) -> void;

        /**
         * @brief Returns the current page (1-based).
         * @return The current page index.
         */
        [[nodiscard]] auto get_current_page() const -> int;

        /**
         * @brief Returns the total number of pages based on the current filter and items per page.
         * @return The total number of pages.
         */
        [[nodiscard]] auto get_total_pages() const -> int;

        /**
         * @brief Returns the number of items to display per page.
         * @return The number of items per page.
         */
        [[nodiscard]] auto get_items_per_page() const -> int;

        /**
         * @brief Returns whether paging is enabled.
         * @return True if paging is enabled, false otherwise.
         */
        [[nodiscard]] auto is_paging_enabled() const -> bool;

        /**
         * @brief Sets the source model for this proxy model.
         * @param sourceModel The source model to filter.
         */
        void setSourceModel(QAbstractItemModel* sourceModel) override;

    protected:
        /**
         * @brief Determines whether the given row should be included in the filtered model and
         * current page.
         * @param source_row The row in the source model.
         * @param source_parent The parent index in the source model.
         * @return True if the row matches all filters and is in the current page, false otherwise.
         */
        auto filterAcceptsRow(int source_row,
                              const QModelIndex& source_parent) const -> bool override;

    private:
        /**
         * @brief Recalculates paging and invalidates the filter.
         */
        auto recalc_paging() -> void;

        /**
         * @brief Checks if a specific row passes the current filters.
         */
        [[nodiscard]] auto row_passes_filter(int row, const QModelIndex& parent) const -> bool;

    private:
        // Filter
        QString m_app_name_filter;
        QSet<QString> m_level_filter;
        QString m_search_text;
        QString m_search_field;
        bool m_use_regex = false;
        QRegularExpression m_search_regex;

        // Paging
        bool m_paging_enabled = true;
        int m_page_size = 25;
        int m_current_page = 1;
        mutable int m_total_passed = 0;
        mutable int m_total_added = 0;
        int m_total_pages = 1;
};
