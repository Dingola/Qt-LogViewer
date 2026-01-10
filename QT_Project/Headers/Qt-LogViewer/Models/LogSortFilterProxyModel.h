#pragma once

/**
 * @file LogSortFilterProxyModel.h
 * @brief This file contains the definition of the LogSortFilterProxyModel class.
 */

#include <QAbstractItemModel>
#include <QCollator>
#include <QHash>
#include <QMap>
#include <QPair>
#include <QRegularExpression>
#include <QSet>
#include <QSortFilterProxyModel>
#include <QString>
#include <QVector>

/**
 * @class LogSortFilterProxyModel
 * @brief Proxy model for filtering and sorting log entries in the LogModel.
 *
 * Supports filtering by application name, log level, search string (plain or regex),
 * custom sorting (e.g., log levels, timestamps), and per-file filters (show-only and hide).
 *
 * Additionally this proxy computes and exposes match ranges for the active search text so
 * delegates can perform lightweight highlighting without containing any search logic.
 */
class LogSortFilterProxyModel: public QSortFilterProxyModel
{
        Q_OBJECT

    public:
        /**
         * @brief Role used to request highlight ranges for a cell.
         *
         * The role returns a QVariantList of QVariantMaps. Each map has keys:
         * - "start" (int) : index of first matching character in the cell text (source text).
         * - "length" (int): number of characters that match.
         *
         * The indexes are relative to the source model string for that cell.
         */
        enum CustomRoles
        {
            HighlightRangesRole = Qt::UserRole + 1
        };

        /**
         * @brief Constructs a LogSortFilterProxyModel object.
         * @param parent The parent QObject.
         */
        explicit LogSortFilterProxyModel(QObject* parent = nullptr);

        /**
         * @brief Sets the application name filter.
         * @param app_name The application name to filter by (empty for no filter).
         */
        auto set_app_name_filter(const QString& app_name) -> void;

        /**
         * @brief Sets the set of log levels to filter by.
         * @param levels The set of log levels (e.g., {"Info", "Error"}).
         */
        auto set_log_level_filters(const QSet<QString>& levels) -> void;

        /**
         * @brief Sets the search string and field.
         * @param search_text The text or regex to search for.
         * @param field The field to search in ("Message", "Level", "AppName", etc.).
         * @param use_regex Whether to interpret search_text as a regular expression.
         */
        auto set_search_filter(const QString& search_text, const QString& field,
                               bool use_regex) -> void;

        /**
         * @brief Sets an optional "show only this file" filter.
         *        When set, only rows from this absolute file path are shown.
         *        Pass empty string to clear the show-only filter.
         * @param file_path Absolute file path to show exclusively, or empty to clear.
         */
        auto set_show_only_file_path(const QString& file_path) -> void;

        /**
         * @brief Hides (excludes) a file from the view. Call again with the same path to keep
         * hidden.
         * @param file_path Absolute file path to hide.
         */
        auto hide_file(const QString& file_path) -> void;

        /**
         * @brief Removes a file from the hidden set (if present).
         * @param file_path Absolute file path to unhide.
         */
        auto unhide_file(const QString& file_path) -> void;

        /**
         * @brief Replaces the entire hidden file set with the provided paths.
         *
         * Emits `file_visibility_changed(QString())` once and invalidates the filter.
         * Passing an empty set clears all hidden files.
         *
         * @param file_paths Set of absolute file paths to hide.
         */
        auto set_hidden_file_paths(const QSet<QString>& file_paths) -> void;

        /**
         * @brief Clears all hidden files.
         */
        auto clear_hidden_files() -> void;

        /**
         * @brief Returns the current application name filter.
         * @return The application name filter string.
         */
        [[nodiscard]] auto get_app_name_filter() const noexcept -> QString;

        /**
         * @brief Returns the current set of log levels being filtered.
         * @return The set of log levels.
         */
        [[nodiscard]] auto get_log_level_filters() const noexcept -> QSet<QString>;

        /**
         * @brief Returns the current search text.
         * @return The search text string.
         */
        [[nodiscard]] auto get_search_text() const noexcept -> QString;

        /**
         * @brief Returns the current search field.
         * @return The search field string.
         */
        [[nodiscard]] auto get_search_field() const noexcept -> QString;

        /**
         * @brief Returns the internal collator used for string comparisons in sorting.
         * @return Reference to the collator.
         */
        [[nodiscard]] auto get_collator() const noexcept -> const QCollator&;

        /**
         * @brief Returns whether the search text is treated as a regex.
         * @return True if using regex, false if plain text.
         */
        [[nodiscard]] auto is_search_regex() const noexcept -> bool;

        /**
         * @brief Indicates whether any filter (app, level, search, file) is currently active.
         * @return True if at least one filter is active.
         */
        [[nodiscard]] auto has_active_filters() const noexcept -> bool;

        /**
         * @brief Returns the current show-only file path.
         * @return Absolute file path, or empty if disabled.
         */
        [[nodiscard]] auto get_show_only_file_path() const noexcept -> QString;

        /**
         * @brief Returns the current set of hidden file paths.
         * @return Set of absolute file paths hidden by this proxy.
         */
        [[nodiscard]] auto get_hidden_file_paths() const noexcept -> QSet<QString>;

        /**
         * @brief Returns the current sort column.
         * @return Column index, or -1 if unsorted.
         */
        [[nodiscard]] auto get_sort_column() const noexcept -> int;

        /**
         * @brief Returns the current sort order.
         * @return Qt::SortOrder currently applied.
         */
        [[nodiscard]] auto get_sort_order() const noexcept -> Qt::SortOrder;

    protected:
        /**
         * @brief Determines whether the given row should be included in the filtered model.
         * @param source_row The row in the source model.
         * @param source_parent The parent index in the source model.
         * @return True if the row matches all filters, false otherwise.
         */
        [[nodiscard]] auto filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
            -> bool override;

        /**
         * @brief Custom sorting logic for columns.
         * @param source_left The left index to compare.
         * @param source_right The right index to compare.
         * @return True if the left value is less than the right value.
         */
        [[nodiscard]] auto lessThan(const QModelIndex& source_left,
                                    const QModelIndex& source_right) const -> bool override;

        /**
         * @brief Intercept data() calls to provide highlight ranges via
         *        `HighlightRangesRole`.
         *
         * Delegates should request `HighlightRangesRole` on the proxy index to obtain a
         * QVariantList of QVariantMap entries with keys "start" and "length".
         */
        [[nodiscard]] auto data(const QModelIndex& index,
                                int role = Qt::DisplayRole) const -> QVariant override;

    private:
        /**
         * @brief Checks if a specific row passes the current filters.
         * @param row The row in the source model.
         * @param parent The parent index in the source model.
         */
        [[nodiscard]] auto row_passes_filter(int row, const QModelIndex& parent) const -> bool;

        /**
         * @brief Recomputes the internal flag indicating active filters.
         */
        auto recalc_active_filters() -> void;

    signals:
        /**
         * @brief Emitted after a file's explicit visibility changed (hidden/unhidden).
         * @param file_path The affected file path.
         */
        void file_visibility_changed(const QString& file_path);

        /**
         * @brief Emitted after the show-only target changed (activated or reset).
         * @param file_path The new show-only target (empty when reset).
         */
        void show_only_changed(const QString& file_path);

    private:
        /**
         * @brief Cached collator used in `lessThan` for case-insensitive string comparison.
         * Reusing a collator avoids repeated construction costs in tight sort loops.
         */
        mutable QCollator m_collator;
        QString m_app_name_filter;
        QSet<QString> m_log_level_filters;
        QString m_search_text;
        QString m_search_field;
        bool m_use_regex = false;
        QRegularExpression m_search_regex;
        bool m_any_filter_active = false;
        QString m_show_only_file_path;
        QSet<QString> m_hidden_file_paths;
        // Highlight cache: source_row -> (source_column -> vector of (start,length))
        QHash<int, QMap<int, QVector<QPair<int, int>>>> m_highlight_map;
};
