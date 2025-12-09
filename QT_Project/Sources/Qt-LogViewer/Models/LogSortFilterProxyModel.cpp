/**
 * @file LogSortFilterProxyModel.cpp
 * @brief This file contains the implementation of the LogSortFilterProxyModel class.
 */

#include "Qt-LogViewer/Models/LogSortFilterProxyModel.h"

#include <QCollator>

#include "Qt-LogViewer/Models/LogEntry.h"
#include "Qt-LogViewer/Models/LogModel.h"

/**
 * @brief Constructs a LogSortFilterProxyModel object.
 * @param parent The parent QObject.
 */
LogSortFilterProxyModel::LogSortFilterProxyModel(QObject* parent): QSortFilterProxyModel(parent)
{
    setSortRole(Qt::DisplayRole);
    setDynamicSortFilter(true);

    m_collator.setCaseSensitivity(Qt::CaseInsensitive);
}

/**
 * @brief Sets the application name filter.
 * @param app_name The application name to filter by (empty for no filter).
 */
auto LogSortFilterProxyModel::set_app_name_filter(const QString& app_name) -> void
{
    if (m_app_name_filter != app_name)
    {
        m_app_name_filter = app_name;
        recalc_active_filters();
        invalidateFilter();
    }
}

/**
 * @brief Sets the set of log levels to filter by.
 * @param levels The set of log levels (e.g., {"Info", "Error"}).
 */
auto LogSortFilterProxyModel::set_log_level_filters(const QSet<QString>& levels) -> void
{
    QSet<QString> normalized_filters;
    for (const auto& filter_level: levels)
    {
        normalized_filters.insert(filter_level.trimmed().toLower());
    }

    if (m_log_level_filters != normalized_filters)
    {
        m_log_level_filters = normalized_filters;
        recalc_active_filters();
        invalidateFilter();
    }
}

/**
 * @brief Sets the search string and field.
 * @param search_text The text or regex to search for.
 * @param field The field to search in ("Message", "Level", "AppName", etc.).
 * @param use_regex Whether to interpret search_text as a regular expression.
 */
auto LogSortFilterProxyModel::set_search_filter(const QString& search_text, const QString& field,
                                                bool use_regex) -> void
{
    bool changed =
        (m_search_text != search_text) || (m_search_field != field) || (m_use_regex != use_regex);

    if (changed)
    {
        m_search_text = search_text;
        m_search_field = field;
        m_use_regex = use_regex;

        if (m_use_regex && !m_search_text.isEmpty())
        {
            m_search_regex =
                QRegularExpression(m_search_text, QRegularExpression::CaseInsensitiveOption);
        }
        else
        {
            m_search_regex = QRegularExpression();
        }

        recalc_active_filters();
        invalidateFilter();
    }
}

/**
 * @brief Sets an optional "show only this file" filter for the proxy.
 * @param file_path Absolute file path to show exclusively, or empty to clear.
 */
auto LogSortFilterProxyModel::set_show_only_file_path(const QString& file_path) -> void
{
    QString normalized = file_path;
    if (m_show_only_file_path != normalized)
    {
        m_show_only_file_path = normalized;
        recalc_active_filters();
        invalidateFilter();
        emit show_only_changed(file_path);
    }
}

/**
 * @brief Hides (excludes) a file from the proxy.
 * @param file_path Absolute file path to hide.
 */
auto LogSortFilterProxyModel::hide_file(const QString& file_path) -> void
{
    if (!file_path.isEmpty())
    {
        if (!m_hidden_file_paths.contains(file_path))
        {
            m_hidden_file_paths.insert(file_path);
            recalc_active_filters();
            invalidateFilter();
            emit file_visibility_changed(file_path);
        }
    }
}

/**
 * @brief Removes a file from the hidden set.
 * @param file_path Absolute file path to unhide.
 */
auto LogSortFilterProxyModel::unhide_file(const QString& file_path) -> void
{
    if (m_hidden_file_paths.contains(file_path))
    {
        m_hidden_file_paths.remove(file_path);
        recalc_active_filters();
        invalidateFilter();
        emit file_visibility_changed(file_path);
    }
}

/**
 * @brief Replaces the entire hidden file set with the provided paths.
 *
 * Emits `file_visibility_changed(QString())` once and invalidates the filter.
 * Passing an empty set clears all hidden files.
 *
 * @param file_paths Set of absolute file paths to hide.
 */
auto LogSortFilterProxyModel::set_hidden_file_paths(const QSet<QString>& file_paths) -> void
{
    const bool changed = (m_hidden_file_paths != file_paths);

    if (changed)
    {
        m_hidden_file_paths = file_paths;
        recalc_active_filters();
        invalidateFilter();
        emit file_visibility_changed(QString());
    }
}

/**
 * @brief Clears all hidden file paths.
 */
auto LogSortFilterProxyModel::clear_hidden_files() -> void
{
    if (!m_hidden_file_paths.isEmpty())
    {
        m_hidden_file_paths.clear();
        recalc_active_filters();
        invalidateFilter();
        emit file_visibility_changed(QString());
    }
}

/**
 * @brief Returns the current application name filter.
 * @return The application name filter string.
 */
auto LogSortFilterProxyModel::get_app_name_filter() const noexcept -> QString
{
    QString value = m_app_name_filter;
    return value;
}

/**
 * @brief Returns the current set of log levels being filtered.
 * @return The set of log levels.
 */
auto LogSortFilterProxyModel::get_log_level_filters() const noexcept -> QSet<QString>
{
    QSet<QString> levels = m_log_level_filters;
    return levels;
}

/**
 * @brief Returns the current search text.
 * @return The search text string.
 */
auto LogSortFilterProxyModel::get_search_text() const noexcept -> QString
{
    QString value = m_search_text;
    return value;
}

/**
 * @brief Returns the current search field.
 * @return The search field string.
 */
auto LogSortFilterProxyModel::get_search_field() const noexcept -> QString
{
    QString value = m_search_field;
    return value;
}

/**
 * @brief Returns the internal collator used for string comparisons in sorting.
 * @return Reference to the collator.
 */
auto LogSortFilterProxyModel::get_collator() const noexcept -> const QCollator&
{
    return m_collator;
}

/**
 * @brief Returns whether the search text is treated as a regex.
 * @return True if using regex, false if plain text.
 */
auto LogSortFilterProxyModel::is_search_regex() const noexcept -> bool
{
    bool value = m_use_regex;
    return value;
}

/**
 * @brief Indicates whether any filter (app, level, search, file) is currently active.
 * @return True if at least one filter is active.
 */
auto LogSortFilterProxyModel::has_active_filters() const noexcept -> bool
{
    bool value = m_any_filter_active;
    return value;
}

/**
 * @brief Returns the current show-only file path.
 * @return Absolute file path, or empty if disabled.
 */
auto LogSortFilterProxyModel::get_show_only_file_path() const noexcept -> QString
{
    QString value = m_show_only_file_path;
    return value;
}

/**
 * @brief Returns the current set of hidden file paths.
 * @return Set of absolute file paths hidden by this proxy.
 */
auto LogSortFilterProxyModel::get_hidden_file_paths() const noexcept -> QSet<QString>
{
    QSet<QString> value = m_hidden_file_paths;
    return value;
}

/**
 * @brief Determines whether the given row should be included in the filtered model.
 * @param source_row The row in the source model.
 * @param source_parent The parent index in the source model.
 * @return True if the row matches all filters, false otherwise.
 */
auto LogSortFilterProxyModel::filterAcceptsRow(int source_row,
                                               const QModelIndex& source_parent) const -> bool
{
    bool accepted = row_passes_filter(source_row, source_parent);
    return accepted;
}

/**
 * @brief Custom sorting logic for columns.
 * @param source_left The left index to compare.
 * @param source_right The right index to compare.
 * @return True if the left value is less than the right value.
 */
auto LogSortFilterProxyModel::lessThan(const QModelIndex& source_left,
                                       const QModelIndex& source_right) const -> bool
{
    bool is_less = false;
    const bool both_timestamp_columns = (source_left.column() == LogModel::Timestamp) &&
                                        (source_right.column() == LogModel::Timestamp);

    if (both_timestamp_columns)
    {
        const QVariant left_value_variant = sourceModel()->data(source_left, Qt::DisplayRole);
        const QVariant right_value_variant = sourceModel()->data(source_right, Qt::DisplayRole);

        const QDateTime left_datetime = left_value_variant.toDateTime();
        const QDateTime right_datetime = right_value_variant.toDateTime();

        if (left_datetime.isValid() && right_datetime.isValid())
        {
            is_less = (left_datetime < right_datetime);
        }
        else
        {
            const QString left_string =
                sourceModel()->data(source_left, Qt::DisplayRole).toString();
            const QString right_string =
                sourceModel()->data(source_right, Qt::DisplayRole).toString();

            is_less = (m_collator.compare(left_string, right_string) < 0);
        }
    }
    else
    {
        const QString left_string = sourceModel()->data(source_left, Qt::DisplayRole).toString();
        const QString right_string = sourceModel()->data(source_right, Qt::DisplayRole).toString();

        is_less = (m_collator.compare(left_string, right_string) < 0);
    }

    return is_less;
}

/**
 * @brief Checks if a specific row passes the current filters, including file filters.
 */
auto LogSortFilterProxyModel::row_passes_filter(int row, const QModelIndex& parent) const -> bool
{
    bool accepted = true;

    // Per-file filter (needs file path from the source model entry)
    QString file_path;
    const auto* log_model = qobject_cast<const LogModel*>(sourceModel());
    if (log_model != nullptr)
    {
        if (row >= 0 && row < log_model->rowCount())
        {
            const LogEntry entry = log_model->get_entry(row);
            file_path = entry.get_file_info().get_file_path();
        }
    }

    if (!m_show_only_file_path.isEmpty())
    {
        if (file_path != m_show_only_file_path)
        {
            accepted = false;
        }
    }
    if (accepted && !m_hidden_file_paths.isEmpty())
    {
        if (m_hidden_file_paths.contains(file_path))
        {
            accepted = false;
        }
    }

    if (accepted)
    {
        if (!m_any_filter_active)
        {
            accepted = true;
        }
        else
        {
            QModelIndex index_app = sourceModel()->index(row, LogModel::AppName, parent);
            QModelIndex index_level = sourceModel()->index(row, LogModel::Level, parent);
            QModelIndex index_message = sourceModel()->index(row, LogModel::Message, parent);

            // App name filter
            if (!m_app_name_filter.isEmpty())
            {
                const QString app_name = sourceModel()->data(index_app, Qt::DisplayRole).toString();
                if (app_name != m_app_name_filter)
                {
                    accepted = false;
                }
            }

            // Level filter
            if (accepted && !m_log_level_filters.isEmpty())
            {
                const QString level = sourceModel()->data(index_level, Qt::DisplayRole).toString();
                const QString normalized_level = level.trimmed().toLower();
                if (!m_log_level_filters.contains(normalized_level))
                {
                    accepted = false;
                }
            }

            // Search filter
            if (accepted && !m_search_text.isEmpty())
            {
                const bool all_fields =
                    (m_search_field.compare("All Fields", Qt::CaseInsensitive) == 0);
                const QString message_value =
                    sourceModel()->data(index_message, Qt::DisplayRole).toString();
                const QString level_value =
                    sourceModel()->data(index_level, Qt::DisplayRole).toString();
                const QString app_value =
                    sourceModel()->data(index_app, Qt::DisplayRole).toString();

                if (m_use_regex)
                {
                    if (!m_search_regex.isValid())
                    {
                        accepted = false;
                    }
                    else if (all_fields)
                    {
                        if (!m_search_regex.match(message_value).hasMatch() &&
                            !m_search_regex.match(level_value).hasMatch() &&
                            !m_search_regex.match(app_value).hasMatch())
                        {
                            accepted = false;
                        }
                    }
                    else if (m_search_field.compare("Message", Qt::CaseInsensitive) == 0)
                    {
                        if (!m_search_regex.match(message_value).hasMatch())
                        {
                            accepted = false;
                        }
                    }
                    else if (m_search_field.compare("Level", Qt::CaseInsensitive) == 0)
                    {
                        if (!m_search_regex.match(level_value).hasMatch())
                        {
                            accepted = false;
                        }
                    }
                    else if (m_search_field.compare("AppName", Qt::CaseInsensitive) == 0)
                    {
                        if (!m_search_regex.match(app_value).hasMatch())
                        {
                            accepted = false;
                        }
                    }
                    else
                    {
                        if (!m_search_regex.match(message_value).hasMatch() &&
                            !m_search_regex.match(level_value).hasMatch() &&
                            !m_search_regex.match(app_value).hasMatch())
                        {
                            accepted = false;
                        }
                    }
                }
                else
                {
                    if (all_fields)
                    {
                        if (!message_value.contains(m_search_text, Qt::CaseInsensitive) &&
                            !level_value.contains(m_search_text, Qt::CaseInsensitive) &&
                            !app_value.contains(m_search_text, Qt::CaseInsensitive))
                        {
                            accepted = false;
                        }
                    }
                    else if (m_search_field.compare("Message", Qt::CaseInsensitive) == 0)
                    {
                        if (!message_value.contains(m_search_text, Qt::CaseInsensitive))
                        {
                            accepted = false;
                        }
                    }
                    else if (m_search_field.compare("Level", Qt::CaseInsensitive) == 0)
                    {
                        if (!level_value.contains(m_search_text, Qt::CaseInsensitive))
                        {
                            accepted = false;
                        }
                    }
                    else if (m_search_field.compare("AppName", Qt::CaseInsensitive) == 0)
                    {
                        if (!app_value.contains(m_search_text, Qt::CaseInsensitive))
                        {
                            accepted = false;
                        }
                    }
                    else
                    {
                        if (!message_value.contains(m_search_text, Qt::CaseInsensitive) &&
                            !level_value.contains(m_search_text, Qt::CaseInsensitive) &&
                            !app_value.contains(m_search_text, Qt::CaseInsensitive))
                        {
                            accepted = false;
                        }
                    }
                }
            }
        }
    }

    return accepted;
}

/**
 * @brief Recomputes the internal flag indicating active filters.
 */
auto LogSortFilterProxyModel::recalc_active_filters() -> void
{
    m_any_filter_active = !m_app_name_filter.isEmpty() || !m_log_level_filters.isEmpty() ||
                          !m_search_text.isEmpty() || !m_show_only_file_path.isEmpty() ||
                          !m_hidden_file_paths.isEmpty();
}
