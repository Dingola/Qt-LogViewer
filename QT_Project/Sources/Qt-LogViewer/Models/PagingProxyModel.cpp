/**
 * @file PagingProxyModel.cpp
 * @brief Implementation of the PagingProxyModel class.
 */

#include "Qt-LogViewer/Models/PagingProxyModel.h"

#include <QSortFilterProxyModel>
#include <algorithm>

/**
 * @brief Constructs a PagingProxyModel.
 * @param parent The parent QObject.
 */
PagingProxyModel::PagingProxyModel(QObject* parent): QAbstractProxyModel(parent) {}

/**
 * @brief Enables or disables paging.
 * @param enabled True to enable paging, false to show all rows.
 */
auto PagingProxyModel::set_paging_enabled(bool enabled) -> void
{
    if (m_paging_enabled != enabled)
    {
        m_paging_enabled = enabled;
        validate_current_page();
        beginResetModel();
        endResetModel();
    }
}

/**
 * @brief Returns whether paging is enabled.
 * @return True if paging is enabled, false otherwise.
 */
auto PagingProxyModel::is_paging_enabled() const -> bool
{
    return m_paging_enabled;
}

/**
 * @brief Sets the number of items per page.
 * @param size The number of items per page (must be > 0).
 */
auto PagingProxyModel::set_page_size(int size) -> void
{
    if (size > 0)
    {
        if (m_page_size != size)
        {
            m_page_size = size;
            validate_current_page();
            beginResetModel();
            endResetModel();
        }
    }
}

/**
 * @brief Returns the number of items per page.
 * @return The number of items per page.
 */
auto PagingProxyModel::get_page_size() const -> int
{
    return m_page_size;
}

/**
 * @brief Sets the current page (1-based).
 * @param page The page number to display.
 */
auto PagingProxyModel::set_current_page(int page) -> void
{
    int new_page = page;

    if (new_page < 1)
    {
        new_page = 1;
    }

    if (m_current_page != new_page)
    {
        m_current_page = new_page;
        validate_current_page();
        beginResetModel();
        endResetModel();
    }
}

/**
 * @brief Returns the current page (1-based).
 * @return The current page number.
 */
auto PagingProxyModel::get_current_page() const -> int
{
    return m_current_page;
}

/**
 * @brief Returns the total number of pages based on the current filter and items per page.
 * @return The total number of pages.
 */
auto PagingProxyModel::get_total_pages() const -> int
{
    int result = 1;

    if (sourceModel() != nullptr)
    {
        int total_rows = sourceModel()->rowCount(QModelIndex());
        int pages = m_paging_enabled ? ((total_rows + m_page_size - 1) / m_page_size) : 1;

        if (pages > 1)
        {
            result = pages;
        }
    }

    return result;
}

/**
 * @brief Sets the source model for this proxy model.
 * @param source_model The source model to page.
 */
void PagingProxyModel::setSourceModel(QAbstractItemModel* source_model)
{
    if (sourceModel() != nullptr)
    {
        disconnect(sourceModel(), nullptr, this, nullptr);
    }

    QAbstractProxyModel::setSourceModel(source_model);

    if (source_model != nullptr)
    {
        connect(source_model, &QAbstractItemModel::dataChanged, this,
                &PagingProxyModel::on_source_data_changed);

        connect(source_model, &QAbstractItemModel::rowsAboutToBeInserted, this,
                &PagingProxyModel::on_source_rows_about_to_be_inserted);

        connect(source_model, &QAbstractItemModel::rowsInserted, this,
                &PagingProxyModel::on_source_rows_inserted);

        connect(source_model, &QAbstractItemModel::rowsAboutToBeRemoved, this,
                &PagingProxyModel::on_source_rows_about_to_be_removed);

        connect(source_model, &QAbstractItemModel::rowsRemoved, this,
                &PagingProxyModel::on_source_rows_removed);

        connect(source_model, &QAbstractItemModel::layoutChanged, this,
                &PagingProxyModel::on_source_layout_changed);

        connect(source_model, &QAbstractItemModel::modelAboutToBeReset, this,
                &PagingProxyModel::on_source_model_about_to_be_reset);

        connect(source_model, &QAbstractItemModel::modelReset, this,
                &PagingProxyModel::on_source_model_reset);
    }
}

/**
 * @brief Returns the number of rows for the current page.
 * @param parent The parent index (usually invalid).
 * @return The number of rows in the current page.
 */
auto PagingProxyModel::rowCount(const QModelIndex& parent) const -> int
{
    int result = 0;

    if (!parent.isValid())
    {
        if (sourceModel() != nullptr)
        {
            int total_rows = sourceModel()->rowCount(QModelIndex());

            if (!m_paging_enabled)
            {
                result = total_rows;
            }
            else
            {
                int offset = get_page_offset();
                int page_rows = std::min(m_page_size, total_rows - offset);

                if (page_rows > 0)
                {
                    result = page_rows;
                }
            }
        }
    }

    return result;
}

/**
 * @brief Returns the number of columns for the current page.
 * @param parent The parent index (usually invalid).
 * @return The number of columns.
 */
auto PagingProxyModel::columnCount(const QModelIndex& parent) const -> int
{
    int result = 0;

    if (sourceModel() != nullptr)
    {
        result = sourceModel()->columnCount(parent);
    }

    return result;
}

/**
 * @brief Returns the index in the proxy model for the given row and column.
 * @param row The row in the proxy model.
 * @param column The column in the proxy model.
 * @param parent The parent index (usually invalid).
 * @return The proxy model index.
 */
auto PagingProxyModel::index(int row, int column, const QModelIndex& parent) const -> QModelIndex
{
    QModelIndex result;

    if (hasIndex(row, column, parent))
    {
        result = createIndex(row, column);
    }

    return result;
}

/**
 * @brief Returns the parent index for the given proxy index.
 * @param index The proxy model index.
 * @return The parent index (always invalid for a flat table).
 */
auto PagingProxyModel::parent(const QModelIndex& /*index*/) const -> QModelIndex
{
    return {};
}

/**
 * @brief Returns the data for the given proxy index and role.
 * @param index The proxy model index.
 * @param role The data role.
 * @return The data value for the given index and role.
 */
auto PagingProxyModel::data(const QModelIndex& index, int role) const -> QVariant
{
    QVariant result;
    QModelIndex src = mapToSource(index);

    if (src.isValid())
    {
        result = sourceModel()->data(src, role);
    }

    return result;
}

/**
 * @brief Maps a proxy index to the corresponding source index.
 * @param proxyIndex The index in the proxy model.
 * @return The corresponding index in the source model.
 */
auto PagingProxyModel::mapToSource(const QModelIndex& proxyIndex) const -> QModelIndex
{
    QModelIndex result;

    if (proxyIndex.isValid())
    {
        if (sourceModel() != nullptr)
        {
            int source_row = proxyIndex.row() + get_page_offset();

            if (source_row < sourceModel()->rowCount(QModelIndex()))
            {
                result = sourceModel()->index(source_row, proxyIndex.column());
            }
        }
    }

    return result;
}

/**
 * @brief Maps a source index to the corresponding proxy index.
 * @param sourceIndex The index in the source model.
 * @return The corresponding index in the proxy model.
 */
auto PagingProxyModel::mapFromSource(const QModelIndex& sourceIndex) const -> QModelIndex
{
    QModelIndex result;

    if (sourceIndex.isValid())
    {
        if (sourceModel() != nullptr)
        {
            int proxy_row = sourceIndex.row() - get_page_offset();

            if (proxy_row >= 0)
            {
                if (proxy_row < rowCount(QModelIndex()))
                {
                    result = index(proxy_row, sourceIndex.column());
                }
            }
        }
    }

    return result;
}

/**
 * @brief Forwards sorting requests to the source model.
 * @param column The column to sort by.
 * @param order The sort order (ascending or descending).
 */
void PagingProxyModel::sort(int column, Qt::SortOrder order)
{
    if (sourceModel() != nullptr)
    {
        auto sort_proxy = qobject_cast<QSortFilterProxyModel*>(sourceModel());

        if (sort_proxy != nullptr)
        {
            sort_proxy->sort(column, order);
        }
    }
}

/**
 * @brief Returns the row offset for the current page.
 * @return The offset (first row index of the current page).
 */
auto PagingProxyModel::get_page_offset() const -> int
{
    int result = 0;

    if (m_paging_enabled)
    {
        result = (m_current_page - 1) * m_page_size;
    }

    return result;
}

/**
 * @brief Ensures the current page is within valid bounds.
 */
auto PagingProxyModel::validate_current_page() -> void
{
    int total_pages = get_total_pages();

    if (m_current_page < 1)
    {
        m_current_page = 1;
    }

    if (m_current_page > total_pages)
    {
        m_current_page = total_pages;
    }
}

/**
 * @brief Forwards source data changes only for rows visible on the active page.
 * @param top_left Top-left source index of the changed range.
 * @param bottom_right Bottom-right source index of the changed range.
 * @param roles List of changed roles.
 */
void PagingProxyModel::on_source_data_changed(const QModelIndex& top_left,
                                              const QModelIndex& bottom_right,
                                              const QList<int>& roles)
{
    if (sourceModel() != nullptr)
    {
        if (!top_left.parent().isValid() && !bottom_right.parent().isValid())
        {
            int visible_rows = rowCount(QModelIndex());
            int visible_columns = columnCount(QModelIndex());

            if (visible_rows > 0 && visible_columns > 0)
            {
                int visible_start = get_page_offset();
                int visible_end = visible_start + visible_rows - 1;
                int changed_start = std::max(top_left.row(), visible_start);
                int changed_end = std::min(bottom_right.row(), visible_end);

                if (changed_start <= changed_end)
                {
                    int left_column = std::max(0, top_left.column());
                    int right_column = std::min(bottom_right.column(), visible_columns - 1);

                    if (left_column <= right_column)
                    {
                        int proxy_top_row = changed_start - visible_start;
                        int proxy_bottom_row = changed_end - visible_start;
                        QModelIndex proxy_top_left = index(proxy_top_row, left_column);
                        QModelIndex proxy_bottom_right = index(proxy_bottom_row, right_column);

                        if (proxy_top_left.isValid() && proxy_bottom_right.isValid())
                        {
                            emit dataChanged(proxy_top_left, proxy_bottom_right, roles);
                        }
                    }
                }
            }
        }
    }
}

/**
 * @brief Handles source row insertion begin notification cleanly.
 * @param parent Parent index in source model.
 * @param start First inserted row in source model.
 * @param end Last inserted row in source model.
 */
void PagingProxyModel::on_source_rows_about_to_be_inserted(const QModelIndex& parent, int start,
                                                           int end)
{
    m_pending_insert = false;
    m_pending_remove = false;
    m_pending_reset = false;
    m_pending_visible_data_refresh = false;
    m_pending_previous_page = m_current_page;

    if (sourceModel() != nullptr && !parent.isValid())
    {
        int change_count = end - start + 1;
        int old_total_rows = sourceModel()->rowCount(QModelIndex());
        int old_proxy_rows = calculate_row_count_for_total_rows(old_total_rows);
        int new_proxy_rows = calculate_row_count_for_total_rows(old_total_rows + change_count);

        if (!m_paging_enabled)
        {
            beginInsertRows(QModelIndex(), start, end);
            m_pending_insert = true;
        }
        else
        {
            int visible_start = get_page_offset();
            int grow_count = new_proxy_rows - old_proxy_rows;

            if (start < visible_start)
            {
                m_pending_reset = true;
            }
            else
            {
                if (grow_count > 0)
                {
                    int proxy_first = start - visible_start;

                    if (proxy_first < 0)
                    {
                        proxy_first = 0;
                    }

                    if (proxy_first > old_proxy_rows)
                    {
                        proxy_first = old_proxy_rows;
                    }

                    int proxy_last = proxy_first + grow_count - 1;

                    beginInsertRows(QModelIndex(), proxy_first, proxy_last);
                    m_pending_insert = true;
                }

                int visible_capacity_end = visible_start + m_page_size - 1;

                if (start <= visible_capacity_end)
                {
                    if (grow_count == 0)
                    {
                        m_pending_visible_data_refresh = true;
                    }
                }
            }
        }
    }
}

/**
 * @brief Handles source row insertion completion notification cleanly.
 * @param parent Parent index in source model.
 * @param start First inserted row in source model.
 * @param end Last inserted row in source model.
 */
void PagingProxyModel::on_source_rows_inserted(const QModelIndex& parent, int /*start*/,
                                               int /*end*/)
{
    if (!parent.isValid())
    {
        validate_current_page();

        if (m_pending_insert)
        {
            endInsertRows();
        }

        bool page_changed = m_current_page != m_pending_previous_page;

        if (m_pending_reset || page_changed)
        {
            beginResetModel();
            endResetModel();
        }
        else if (m_pending_visible_data_refresh)
        {
            refresh_visible_page_data();
        }

        m_pending_insert = false;
        m_pending_remove = false;
        m_pending_reset = false;
        m_pending_visible_data_refresh = false;
    }
}

/**
 * @brief Handles source row removal begin notification cleanly.
 * @param parent Parent index in source model.
 * @param start First removed row in source model.
 * @param end Last removed row in source model.
 */
void PagingProxyModel::on_source_rows_about_to_be_removed(const QModelIndex& parent, int start,
                                                          int end)
{
    m_pending_insert = false;
    m_pending_remove = false;
    m_pending_reset = false;
    m_pending_visible_data_refresh = false;
    m_pending_previous_page = m_current_page;

    if (sourceModel() != nullptr && !parent.isValid())
    {
        int change_count = end - start + 1;
        int old_total_rows = sourceModel()->rowCount(QModelIndex());
        int old_proxy_rows = calculate_row_count_for_total_rows(old_total_rows);
        int new_proxy_rows = calculate_row_count_for_total_rows(old_total_rows - change_count);

        if (!m_paging_enabled)
        {
            beginRemoveRows(QModelIndex(), start, end);
            m_pending_remove = true;
        }
        else
        {
            int visible_start = get_page_offset();
            int shrink_count = old_proxy_rows - new_proxy_rows;

            if (start < visible_start)
            {
                m_pending_reset = true;
            }
            else
            {
                if (shrink_count > 0 && old_proxy_rows > 0)
                {
                    int proxy_first = start - visible_start;

                    if (proxy_first < 0)
                    {
                        proxy_first = 0;
                    }

                    if (proxy_first >= old_proxy_rows)
                    {
                        proxy_first = old_proxy_rows - 1;
                    }

                    int proxy_last = proxy_first + shrink_count - 1;

                    beginRemoveRows(QModelIndex(), proxy_first, proxy_last);
                    m_pending_remove = true;
                }

                int visible_end = visible_start + old_proxy_rows - 1;

                if (start <= visible_end)
                {
                    if (shrink_count == 0)
                    {
                        m_pending_visible_data_refresh = true;
                    }
                }
            }
        }
    }
}

/**
 * @brief Handles source row removal completion notification cleanly.
 * @param parent Parent index in source model.
 * @param start First removed row in source model.
 * @param end Last removed row in source model.
 */
void PagingProxyModel::on_source_rows_removed(const QModelIndex& parent, int /*start*/, int /*end*/)
{
    if (!parent.isValid())
    {
        validate_current_page();

        if (m_pending_remove)
        {
            endRemoveRows();
        }

        bool page_changed = m_current_page != m_pending_previous_page;

        if (m_pending_reset || page_changed)
        {
            beginResetModel();
            endResetModel();
        }
        else if (m_pending_visible_data_refresh)
        {
            refresh_visible_page_data();
        }

        m_pending_insert = false;
        m_pending_remove = false;
        m_pending_reset = false;
        m_pending_visible_data_refresh = false;
    }
}

/**
 * @brief Fallback proxy reset when source layout changes.
 */
void PagingProxyModel::on_source_layout_changed()
{
    validate_current_page();
    beginResetModel();
    endResetModel();
}

/**
 * @brief Pre-reset model handle.
 */
void PagingProxyModel::on_source_model_about_to_be_reset()
{
    beginResetModel();
}

/**
 * @brief Fallback proxy reset when source model is reset.
 */
void PagingProxyModel::on_source_model_reset()
{
    validate_current_page();
    endResetModel();
}

/**
 * @brief Returns visible row count for a hypothetical source row count.
 * @param total_rows Source row count to evaluate.
 * @return Visible proxy row count for current page/paging state.
 */
auto PagingProxyModel::calculate_row_count_for_total_rows(int total_rows) const -> int
{
    int result = 0;

    if (!m_paging_enabled)
    {
        if (total_rows > 0)
        {
            result = total_rows;
        }
    }
    else
    {
        int offset = get_page_offset();
        int page_rows = std::min(m_page_size, total_rows - offset);

        if (page_rows > 0)
        {
            result = page_rows;
        }
    }

    return result;
}

/**
 * @brief Emits dataChanged for the full visible page range.
 */
auto PagingProxyModel::refresh_visible_page_data() -> void
{
    int rows = rowCount(QModelIndex());
    int columns = columnCount(QModelIndex());

    if (rows > 0 && columns > 0)
    {
        emit dataChanged(index(0, 0), index(rows - 1, columns - 1), QList<int>());
    }
}