/**
 * @file LogSortFilterProxyModel.cpp
 * @brief This file contains the implementation of the LogSortFilterProxyModel class.
 */

#include "Qt-LogViewer/Models/LogSortFilterProxyModel.h"

#include <QAbstractProxyModel>
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
    m_collator.setNumericMode(true);
}

/**
 * @brief Toggles the ingestion mode to optimize mass inserts.
 * @param active True to pause sorting/filtering for ingestion, false to resume.
 */
auto LogSortFilterProxyModel::set_ingestion_mode(bool active) -> void
{
    if (m_ingestion_active != active)
    {
        m_ingestion_active = active;
        setDynamicSortFilter(!active);

        if (!active)
        {
            // Resuming from ingestion: force a full refilter and resort.
            invalidate();
        }
    }
}

/**
 * @brief Returns whether ingestion mode is currently active.
 * @return True if dynamic sort/filter is suspended.
 */
auto LogSortFilterProxyModel::is_ingestion_mode_active() const noexcept -> bool
{
    bool active = m_ingestion_active;
    return active;
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
 * @param field The field to search in.
 * @param use_regex Whether to interpret search_text as a regular expression.
 */
auto LogSortFilterProxyModel::set_search_filter(const QString& search_text, SearchField field,
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

        if (!m_ingestion_active)
        {
            // Force repaint of all cells for updated highlight ranges.
            // Emit dataChanged for HighlightRangesRole across the entire proxy range.
            const int rows = rowCount();
            const int cols = columnCount();
            if (rows > 0 && cols > 0)
            {
                const QModelIndex top_left = index(0, 0);
                const QModelIndex bottom_right = index(rows - 1, cols - 1);
                emit dataChanged(top_left, bottom_right, {HighlightRangesRole});
            }
        }
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
 * @return The search field.
 */
auto LogSortFilterProxyModel::get_search_field() const noexcept -> SearchField
{
    SearchField value = m_search_field;
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
 * @brief Returns the current sort column.
 * @return Column index, or -1 if unsorted.
 */
auto LogSortFilterProxyModel::get_sort_column() const noexcept -> int
{
    int col = sortColumn();
    return col;
}

/**
 * @brief Returns the current sort order.
 * @return Qt::SortOrder currently applied.
 */
auto LogSortFilterProxyModel::get_sort_order() const noexcept -> Qt::SortOrder
{
    Qt::SortOrder order = sortOrder();
    return order;
}

/**
 * @brief Intercept data() calls to provide highlight ranges via the custom role.
 *
 * Instead of using a pre-computed cache, this computes highlight ranges on-demand
 * for each cell. This ensures ranges are always correct for the current search text.
 *
 * @param index The proxy index to query.
 * @param role The role being requested.
 * @return QVariant carrying either delegate highlight info or a normal role value.
 */
auto LogSortFilterProxyModel::data(const QModelIndex& index, int role) const -> QVariant
{
    QVariant value;

    if (role == HighlightRangesRole)
    {
        if (index.isValid() && !m_search_text.isEmpty())
        {
            const QModelIndex src_index = mapToSource(index);
            const int source_column = src_index.column();

            // Check if this column should be searched
            bool should_check = false;

            switch (m_search_field)
            {
            case SearchField::AllFields:
                should_check = true;
                break;
            case SearchField::Message:
                should_check = (source_column == LogModel::Message);
                break;
            case SearchField::Level:
                should_check = (source_column == LogModel::Level);
                break;
            case SearchField::AppName:
                should_check = (source_column == LogModel::AppName);
                break;
            case SearchField::Count:
                break;
            }

            if (should_check)
            {
                const QString cell_text =
                    QSortFilterProxyModel::data(index, Qt::DisplayRole).toString();

                QVariantList list;

                if (m_use_regex && m_search_regex.isValid())
                {
                    QRegularExpressionMatchIterator it = m_search_regex.globalMatch(cell_text);
                    while (it.hasNext())
                    {
                        QRegularExpressionMatch match = it.next();
                        const int start = match.capturedStart();
                        const int len = match.capturedLength();
                        if (start >= 0 && len > 0)
                        {
                            QVariantMap item;
                            item.insert(QStringLiteral("start"), start);
                            item.insert(QStringLiteral("length"), len);
                            list.append(item);
                        }
                    }
                }
                else
                {
                    const QString lower_text = cell_text.toLower();
                    const QString lower_search = m_search_text.toLower();
                    const int search_len = lower_search.length();
                    int pos = 0;

                    while (pos < lower_text.length())
                    {
                        const int found = lower_text.indexOf(lower_search, pos);
                        if (found >= 0)
                        {
                            QVariantMap item;
                            item.insert(QStringLiteral("start"), found);
                            item.insert(QStringLiteral("length"), search_len);
                            list.append(item);
                            pos = found + qMax(1, search_len);
                        }
                        else
                        {
                            pos = lower_text.length();
                        }
                    }
                }

                if (!list.isEmpty())
                {
                    value = list;
                }
            }
        }
    }
    else
    {
        value = QSortFilterProxyModel::data(index, role);
    }

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
    const int col = source_left.column();

    if (col == LogModel::Timestamp)
    {
        const QDateTime left_dt = sourceModel()->data(source_left, Qt::DisplayRole).toDateTime();
        const QDateTime right_dt = sourceModel()->data(source_right, Qt::DisplayRole).toDateTime();

        if (left_dt.isValid() && right_dt.isValid())
        {
            is_less = (left_dt < right_dt);
        }
        else
        {
            const QString left_str = sourceModel()->data(source_left, Qt::DisplayRole).toString();
            const QString right_str = sourceModel()->data(source_right, Qt::DisplayRole).toString();
            is_less = (m_collator.compare(left_str, right_str) < 0);
        }
    }
    else
    {
        const QString left_str = sourceModel()->data(source_left, Qt::DisplayRole).toString();
        const QString right_str = sourceModel()->data(source_right, Qt::DisplayRole).toString();

        if (col == LogModel::AppName || col == LogModel::Message)
        {
            is_less = (m_collator.compare(left_str, right_str) < 0);
        }
        else
        {
            is_less = (QString::compare(left_str, right_str, Qt::CaseInsensitive) < 0);
        }
    }

    return is_less;
}

/**
 * @brief Checks if a specific row passes the current filters.
 * @param row The row in the source model.
 * @param parent The parent index in the source model.
 */
auto LogSortFilterProxyModel::row_passes_filter(int row, const QModelIndex& parent) const -> bool
{
    bool accepted = true;

    if (m_any_filter_active)
    {
        QAbstractItemModel* src = sourceModel();

        // 1. File Filters
        if (!m_show_only_file_path.isEmpty() || !m_hidden_file_paths.isEmpty())
        {
            const auto* log_model = qobject_cast<const LogModel*>(src);
            if (log_model != nullptr && row >= 0 && row < log_model->rowCount())
            {
                const LogEntry entry = log_model->get_entry(row);
                const QString file_path = entry.get_file_info().get_file_path();

                if (!m_show_only_file_path.isEmpty() && file_path != m_show_only_file_path)
                {
                    accepted = false;
                }

                if (accepted && !m_hidden_file_paths.isEmpty() &&
                    m_hidden_file_paths.contains(file_path))
                {
                    accepted = false;
                }
            }
            else
            {
                accepted = false;
            }
        }

        // 2. App Name Filter
        if (accepted && !m_app_name_filter.isEmpty())
        {
            const QString app =
                src->data(src->index(row, LogModel::AppName, parent), Qt::DisplayRole).toString();
            if (app != m_app_name_filter)
            {
                accepted = false;
            }
        }

        // 3. Level Filter
        if (accepted && !m_log_level_filters.isEmpty())
        {
            const QString level =
                src->data(src->index(row, LogModel::Level, parent), Qt::DisplayRole).toString();
            const QStringView level_view = QStringView{level}.trimmed();

            bool level_matched = false;

            for (const QString& filter_lvl: m_log_level_filters)
            {
                if (level_view.compare(filter_lvl, Qt::CaseInsensitive) == 0)
                {
                    level_matched = true;
                    break;
                }
            }

            if (!level_matched)
            {
                accepted = false;
            }
        }

        // 4. Search Filter (Lazy Evaluation & Short-Circuit)
        if (accepted && !m_search_text.isEmpty())
        {
            const bool is_message = (m_search_field == SearchField::Message);
            const bool is_level = (m_search_field == SearchField::Level);
            const bool is_app = (m_search_field == SearchField::AppName);

            // If the field isn't explicitly recognized, fall back to evaluating all fields
            const bool all_fields = (m_search_field == SearchField::AllFields);

            bool matched = false;

            const auto check_match = [&](const QString& value) -> bool {
                bool found = false;
                if (m_use_regex && m_search_regex.isValid())
                {
                    found = m_search_regex.match(value).hasMatch();
                }
                else if (!m_use_regex)
                {
                    found = value.contains(m_search_text, Qt::CaseInsensitive);
                }
                return found;
            };

            if (all_fields || is_message)
            {
                const QString msg =
                    src->data(src->index(row, LogModel::Message, parent), Qt::DisplayRole)
                        .toString();
                matched = check_match(msg);
            }

            if (!matched && (all_fields || is_level))
            {
                const QString lvl =
                    src->data(src->index(row, LogModel::Level, parent), Qt::DisplayRole).toString();
                matched = check_match(lvl);
            }

            if (!matched && (all_fields || is_app))
            {
                const QString app =
                    src->data(src->index(row, LogModel::AppName, parent), Qt::DisplayRole)
                        .toString();
                matched = check_match(app);
            }

            if (!matched)
            {
                accepted = false;
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
