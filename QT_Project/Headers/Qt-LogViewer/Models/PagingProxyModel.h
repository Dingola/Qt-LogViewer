#pragma once

#include <QAbstractProxyModel>

/**
 * @brief A proxy model that provides paging for any source model.
 *
 * This proxy allows showing only a subset of rows (a "page") from the source model.
 * Paging can be enabled/disabled at runtime.
 */
class PagingProxyModel: public QAbstractProxyModel
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs a PagingProxyModel.
         * @param parent The parent QObject.
         */
        explicit PagingProxyModel(QObject* parent = nullptr);

        /**
         * @brief Enables or disables paging.
         * @param enabled True to enable paging, false to show all rows.
         */
        auto set_paging_enabled(bool enabled) -> void;

        /**
         * @brief Returns whether paging is enabled.
         * @return True if paging is enabled, false otherwise.
         */
        [[nodiscard]] auto is_paging_enabled() const -> bool;

        /**
         * @brief Sets the number of items per page.
         * @param size The number of items per page (must be > 0).
         */
        auto set_page_size(int size) -> void;

        /**
         * @brief Returns the number of items per page.
         * @return The number of items per page.
         */
        [[nodiscard]] auto get_page_size() const -> int;

        /**
         * @brief Sets the current page (1-based).
         * @param page The page number to display.
         */
        auto set_current_page(int page) -> void;

        /**
         * @brief Returns the current page (1-based).
         * @return The current page number.
         */
        [[nodiscard]] auto get_current_page() const -> int;

        /**
         * @brief Returns the total number of pages based on the current filter and items per page.
         * @return The total number of pages.
         */
        [[nodiscard]] auto get_total_pages() const -> int;

        /**
         * @brief Sets the source model for this proxy model.
         * @param source_model The source model to page.
         */
        void setSourceModel(QAbstractItemModel* source_model) override;

        /**
         * @brief Returns the number of rows for the current page.
         * @param parent The parent index (usually invalid).
         * @return The number of rows in the current page.
         */
        [[nodiscard]] auto rowCount(const QModelIndex& parent = QModelIndex()) const
            -> int override;

        /**
         * @brief Returns the number of columns for the current page.
         * @param parent The parent index (usually invalid).
         * @return The number of columns.
         */
        [[nodiscard]] auto columnCount(const QModelIndex& parent = QModelIndex()) const
            -> int override;

        /**
         * @brief Returns the index in the proxy model for the given row and column.
         * @param row The row in the proxy model.
         * @param column The column in the proxy model.
         * @param parent The parent index (usually invalid).
         * @return The proxy model index.
         */
        [[nodiscard]] auto index(int row, int column, const QModelIndex& parent = QModelIndex())
            const -> QModelIndex override;

        /**
         * @brief Returns the parent index for the given proxy index.
         * @param index The proxy model index.
         * @return The parent index (always invalid for a flat table).
         */
        [[nodiscard]] auto parent(const QModelIndex& index) const -> QModelIndex override;

        /**
         * @brief Returns the data for the given proxy index and role.
         * @param index The proxy model index.
         * @param role The data role.
         * @return The data value for the given index and role.
         */
        [[nodiscard]] auto data(const QModelIndex& index,
                                int role = Qt::DisplayRole) const -> QVariant override;

        /**
         * @brief Maps a proxy index to the corresponding source index.
         * @param proxyIndex The index in the proxy model.
         * @return The corresponding index in the source model.
         */
        [[nodiscard]] auto mapToSource(const QModelIndex& proxyIndex) const -> QModelIndex override;

        /**
         * @brief Maps a source index to the corresponding proxy index.
         * @param sourceIndex The index in the source model.
         * @return The corresponding index in the proxy model.
         */
        [[nodiscard]] auto mapFromSource(const QModelIndex& sourceIndex) const
            -> QModelIndex override;

        /**
         * @brief Forwards sorting requests to the source model.
         * @param column The column to sort by.
         * @param order The sort order (ascending or descending).
         */
        void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    protected:
        /**
         * @brief Returns the row offset for the current page.
         * @return The offset (first row index of the current page).
         */
        [[nodiscard]] auto get_page_offset() const -> int;

        /**
         * @brief Ensures the current page is within valid bounds.
         */
        auto validate_current_page() -> void;

    private slots:
        /**
         * @brief Forwards source data changes only for rows visible on the active page.
         * @param top_left Top-left source index of the changed range.
         * @param bottom_right Bottom-right source index of the changed range.
         * @param roles List of changed roles.
         */
        void on_source_data_changed(const QModelIndex& top_left, const QModelIndex& bottom_right,
                                    const QList<int>& roles);

        /**
         * @brief Handles source row insertion begin notification cleanly.
         * @param parent Parent index in source model.
         * @param start First inserted row in source model.
         * @param end Last inserted row in source model.
         */
        void on_source_rows_about_to_be_inserted(const QModelIndex& parent, int start, int end);

        /**
         * @brief Handles source row insertion completion notification cleanly.
         * @param parent Parent index in source model.
         * @param start First inserted row in source model.
         * @param end Last inserted row in source model.
         */
        void on_source_rows_inserted(const QModelIndex& parent, int start, int end);

        /**
         * @brief Handles source row removal begin notification cleanly.
         * @param parent Parent index in source model.
         * @param start First removed row in source model.
         * @param end Last removed row in source model.
         */
        void on_source_rows_about_to_be_removed(const QModelIndex& parent, int start, int end);

        /**
         * @brief Handles source row removal completion notification cleanly.
         * @param parent Parent index in source model.
         * @param start First removed row in source model.
         * @param end Last removed row in source model.
         */
        void on_source_rows_removed(const QModelIndex& parent, int start, int end);

        /**
         * @brief Fallback proxy reset when source layout changes.
         */
        void on_source_layout_changed();

        /**
         * @brief Pre-reset model handle.
         */
        void on_source_model_about_to_be_reset();

        /**
         * @brief Fallback proxy reset when source model is reset.
         */
        void on_source_model_reset();

    private:
        /**
         * @brief Returns visible row count for a hypothetical source row count.
         * @param total_rows Source row count to evaluate.
         * @return Visible proxy row count for current page/paging state.
         */
        [[nodiscard]] auto calculate_row_count_for_total_rows(int total_rows) const -> int;

        /**
         * @brief Emits dataChanged for the full visible page range.
         */
        auto refresh_visible_page_data() -> void;

    private:
        bool m_paging_enabled = true;
        int m_page_size = 25;
        int m_current_page = 1;

        bool m_pending_insert = false;
        bool m_pending_remove = false;
        bool m_pending_reset = false;
        bool m_pending_visible_data_refresh = false;
        int m_pending_previous_page = 1;
};
