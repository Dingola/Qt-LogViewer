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
        connect(source_model, &QAbstractItemModel::modelReset, this, [this]() {
            validate_current_page();
            beginResetModel();
            endResetModel();
        });
        connect(source_model, &QAbstractItemModel::dataChanged, this, [this]() {
            validate_current_page();
            beginResetModel();
            endResetModel();
        });
        connect(source_model, &QAbstractItemModel::layoutChanged, this, [this]() {
            validate_current_page();
            beginResetModel();
            endResetModel();
        });
        connect(source_model, &QAbstractItemModel::rowsInserted, this, [this]() {
            validate_current_page();
            beginResetModel();
            endResetModel();
        });
        connect(source_model, &QAbstractItemModel::rowsRemoved, this, [this]() {
            validate_current_page();
            beginResetModel();
            endResetModel();
        });
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
    return QModelIndex();
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
        QSortFilterProxyModel* sort_proxy = qobject_cast<QSortFilterProxyModel*>(sourceModel());

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
