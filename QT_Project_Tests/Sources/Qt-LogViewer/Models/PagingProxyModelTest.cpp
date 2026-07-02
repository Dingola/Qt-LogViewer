#include "Qt-LogViewer/Models/PagingProxyModelTest.h"

#include <QSignalSpy>
#include <QSortFilterProxyModel>
#include <QString>
#include <chrono>
#include <iostream>

namespace
{
/**
 * @brief Prints a compact proxy signal matrix for a measured operation.
 */
auto print_proxy_signal_matrix(const char* label, const QSignalSpy& model_reset_spy,
                               const QSignalSpy& rows_about_to_inserted_spy,
                               const QSignalSpy& rows_inserted_spy,
                               const QSignalSpy& rows_about_to_removed_spy,
                               const QSignalSpy& rows_removed_spy,
                               const QSignalSpy& data_changed_spy) -> void
{
    std::cout << "[DIAG] " << label << " proxy signals: "
              << "modelReset=" << model_reset_spy.count()
              << ", rowsAboutToBeInserted=" << rows_about_to_inserted_spy.count()
              << ", rowsInserted=" << rows_inserted_spy.count()
              << ", rowsAboutToBeRemoved=" << rows_about_to_removed_spy.count()
              << ", rowsRemoved=" << rows_removed_spy.count()
              << ", dataChanged=" << data_changed_spy.count() << std::endl;
}
}  // namespace

/**
 * @brief Sets up the test fixture for each test.
 */
void PagingProxyModelTest::SetUp()
{
    m_source = new QStandardItemModel();
    m_source->setColumnCount(1);

    m_proxy = new PagingProxyModel();
    m_proxy->setSourceModel(m_source);
}

/**
 * @brief Tears down the test fixture after each test.
 */
void PagingProxyModelTest::TearDown()
{
    delete m_proxy;
    delete m_source;
    m_proxy = nullptr;
    m_source = nullptr;
}

/**
 * @brief Seeds the source model with sequential rows in column 0.
 * @param row_count Number of rows to create.
 */
auto PagingProxyModelTest::seed_rows(int row_count) -> void
{
    m_source->setRowCount(0);

    if (row_count > 0)
    {
        m_source->insertRows(0, row_count);

        for (int row = 0; row < row_count; ++row)
        {
            const QModelIndex index = m_source->index(row, 0);
            m_source->setData(index, QString("row_%1").arg(row), Qt::DisplayRole);
        }
    }
}

/**
 * @brief Resets source model size quickly for benchmark setup.
 * @param row_count Number of rows to create.
 */
auto PagingProxyModelTest::set_source_row_count(int row_count) -> void
{
    m_source->setRowCount(0);

    if (row_count > 0)
    {
        m_source->insertRows(0, row_count);
    }
}

// --- 1. SETUP / LIFECYCLE TESTS ---

/**
 * @brief Verifies default paging state and first-page visibility.
 */
TEST_F(PagingProxyModelTest, DefaultStateAndFirstPageVisibility)
{
    seed_rows(60);

    EXPECT_TRUE(m_proxy->is_paging_enabled());
    EXPECT_EQ(m_proxy->get_page_size(), 25);
    EXPECT_EQ(m_proxy->get_current_page(), 1);
    EXPECT_EQ(m_proxy->get_total_pages(), 3);
    EXPECT_EQ(m_proxy->rowCount(), 25);
}

/**
 * @brief Verifies class behavior works flawlessly when source is a nullptr.
 */
TEST_F(PagingProxyModelTest, SourceModel_Null_Behaviors)
{
    m_proxy->setSourceModel(nullptr);

    EXPECT_EQ(m_proxy->rowCount(), 0);
    EXPECT_EQ(m_proxy->columnCount(), 0);
    EXPECT_EQ(m_proxy->get_total_pages(), 1);
    EXPECT_FALSE(m_proxy->index(0, 0).isValid());
    EXPECT_FALSE(m_proxy->mapToSource(QModelIndex()).isValid());
    EXPECT_FALSE(m_proxy->mapFromSource(QModelIndex()).isValid());

    // Ensure sorting on nullptr doesn't crash
    m_proxy->sort(0);
}

// --- 2. CONFIGURATION TESTS ---

/**
 * @brief Verifies that setting page size blocks non-positive values.
 */
TEST_F(PagingProxyModelTest, SetPageSize_ValidatesInput)
{
    m_proxy->set_page_size(10);
    EXPECT_EQ(m_proxy->get_page_size(), 10);
    m_proxy->set_page_size(0);
    EXPECT_EQ(m_proxy->get_page_size(), 10);  // Ignoring <= 0
    m_proxy->set_page_size(-5);
    EXPECT_EQ(m_proxy->get_page_size(), 10);  // Ignoring <= 0
}

/**
 * @brief Verifies that setting the current page handles out of bounds.
 */
TEST_F(PagingProxyModelTest, SetCurrentPage_OutOfBounds)
{
    seed_rows(50);
    m_proxy->set_page_size(10);  // 5 pages available

    m_proxy->set_current_page(0);
    EXPECT_EQ(m_proxy->get_current_page(), 1);

    m_proxy->set_current_page(-5);
    EXPECT_EQ(m_proxy->get_current_page(), 1);

    m_proxy->set_current_page(10);
    EXPECT_EQ(m_proxy->get_current_page(), 5);
}

/**
 * @brief Verifies page size and last-page row count behavior.
 */
TEST_F(PagingProxyModelTest, PageSizeAffectsTotalPagesAndLastPageSize)
{
    seed_rows(52);

    m_proxy->set_page_size(20);

    EXPECT_EQ(m_proxy->get_total_pages(), 3);

    m_proxy->set_current_page(3);

    EXPECT_EQ(m_proxy->rowCount(), 12);
}

/**
 * @brief Verifies disabling paging exposes all source rows.
 */
TEST_F(PagingProxyModelTest, DisablePagingShowsAllRows)
{
    seed_rows(60);

    m_proxy->set_paging_enabled(false);

    EXPECT_FALSE(m_proxy->is_paging_enabled());
    EXPECT_EQ(m_proxy->rowCount(), 60);
    EXPECT_EQ(m_proxy->get_total_pages(), 1);
}

// --- 3. DATA ACCESS & MAPPING TESTS ---

/**
 * @brief Verifies Model structured data access methods (columnCount, rowCount, parent, index).
 */
TEST_F(PagingProxyModelTest, ModelStructureMethods)
{
    seed_rows(10);
    m_proxy->set_page_size(5);

    // columnCount
    EXPECT_EQ(m_proxy->columnCount(), 1);
    EXPECT_EQ(m_proxy->columnCount(m_proxy->index(0, 0)), 0);

    // parent
    EXPECT_FALSE(m_proxy->parent(m_proxy->index(0, 0)).isValid());

    // index
    EXPECT_TRUE(m_proxy->index(0, 0).isValid());
    EXPECT_FALSE(m_proxy->index(10, 0).isValid());  // Out of bounds for the page
    EXPECT_FALSE(m_proxy->index(0, 1).isValid());   // Invalid column
}

/**
 * @brief Verifies correct data mapping through the data() method.
 */
TEST_F(PagingProxyModelTest, Data_ValidAndInvalid)
{
    seed_rows(10);
    m_proxy->set_page_size(5);

    QModelIndex valid_index = m_proxy->index(0, 0);
    EXPECT_EQ(m_proxy->data(valid_index).toString(), QString("row_0"));

    QModelIndex invalid_index = m_proxy->index(10, 0);
    EXPECT_FALSE(m_proxy->data(invalid_index).isValid());
}

/**
 * @brief Verifies page navigation maps to the correct source slice.
 */
TEST_F(PagingProxyModelTest, CurrentPageMapsToCorrectSourceRows)
{
    seed_rows(60);

    m_proxy->set_page_size(10);
    m_proxy->set_current_page(2);

    EXPECT_EQ(m_proxy->rowCount(), 10);
    EXPECT_EQ(m_proxy->get_total_pages(), 6);

    const QModelIndex proxy_index = m_proxy->index(0, 0);
    const QModelIndex source_index = m_proxy->mapToSource(proxy_index);

    ASSERT_TRUE(source_index.isValid());
    EXPECT_EQ(source_index.row(), 10);
    EXPECT_EQ(m_source->data(source_index, Qt::DisplayRole).toString(), QString("row_10"));
}

/**
 * @brief Verifies bounding mapping mechanisms (mapToSource and mapFromSource).
 */
TEST_F(PagingProxyModelTest, MapToSource_MapFromSource_Bounds)
{
    seed_rows(20);
    m_proxy->set_page_size(10);
    m_proxy->set_current_page(2);  // Page 2 points to rows 10-19

    // Test mapToSource
    QModelIndex proxy_idx = m_proxy->index(0, 0);
    QModelIndex src_idx = m_proxy->mapToSource(proxy_idx);
    EXPECT_TRUE(src_idx.isValid());
    EXPECT_EQ(src_idx.row(), 10);

    // Test mapFromSource
    QModelIndex valid_src_idx = m_source->index(15, 0);
    QModelIndex mapped_proxy_idx = m_proxy->mapFromSource(valid_src_idx);
    EXPECT_TRUE(mapped_proxy_idx.isValid());
    EXPECT_EQ(mapped_proxy_idx.row(), 5);

    // Test mapFromSource (out of visible page bounds)
    QModelIndex non_visible_src_idx = m_source->index(5, 0);  // On page 1
    EXPECT_FALSE(m_proxy->mapFromSource(non_visible_src_idx).isValid());

    // Invalid source maps to invalid proxy map
    EXPECT_FALSE(m_proxy->mapFromSource(QModelIndex()).isValid());

    // Invalid proxy maps to invalid source map
    EXPECT_FALSE(m_proxy->mapToSource(QModelIndex()).isValid());
}

/**
 * @brief Verifies sort() is properly delegated down to QSortFilterProxyModel.
 */
TEST_F(PagingProxyModelTest, Sort_ForwardsToSortFilterProxy)
{
    seed_rows(10);  // "row_0" through "row_9"

    QSortFilterProxyModel sort_proxy;
    sort_proxy.setSourceModel(m_source);

    m_proxy->setSourceModel(&sort_proxy);
    m_proxy->sort(0, Qt::DescendingOrder);

    QModelIndex first_proxy_idx = m_proxy->index(0, 0);
    EXPECT_EQ(m_proxy->data(first_proxy_idx).toString(),
              QString("row_9"));  // With string sort, "row_9" is on top
}

// --- 4. DYNAMIC DATA UPDATES & SIGNALS TESTS  ---

/**
 * @brief Verifies get_total_pages handles insertion/removal dynamics dynamically.
 */
TEST_F(PagingProxyModelTest, TotalPagesUpdatesOnRowInsertionsAndDecrements)
{
    seed_rows(10);
    m_proxy->set_page_size(10);
    EXPECT_EQ(m_proxy->get_total_pages(), 1);

    m_source->insertRow(10);
    EXPECT_EQ(m_proxy->get_total_pages(), 2);

    m_source->removeRow(10);
    EXPECT_EQ(m_proxy->get_total_pages(), 1);
}

/**
 * @brief Verifies that configuring the proxy triggers reset signals to inform views.
 */
TEST_F(PagingProxyModelTest, PropertyChangesEmitModelReset)
{
    seed_rows(20);
    QSignalSpy reset_spy(m_proxy, &QAbstractItemModel::modelReset);

    m_proxy->set_page_size(10);
    EXPECT_EQ(reset_spy.count(), 1);

    m_proxy->set_current_page(2);
    EXPECT_EQ(reset_spy.count(), 2);

    m_proxy->set_paging_enabled(false);
    EXPECT_EQ(reset_spy.count(), 3);
}

/**
 * @brief Verifies that model resets and layout changes emitted by the source model are mirrored.
 */
TEST_F(PagingProxyModelTest, SourceModelResetsAndLayoutChanges)
{
    seed_rows(10);
    m_proxy->set_page_size(5);

    QSignalSpy reset_spy(m_proxy, &QAbstractItemModel::modelReset);

    // QStandardItemModel::sort(int) intern triggers layoutAboutToBeChanged and layoutChanged
    m_source->sort(0);
    EXPECT_GE(reset_spy.count(), 1);

    reset_spy.clear();

    // QStandardItemModel::clear() intern triggers modelAboutToBeReset and modelReset
    m_source->clear();
    EXPECT_EQ(reset_spy.count(), 1);
}

/**
 * @brief Diagnostic: single append INSIDE visible page range.
 */
TEST_F(PagingProxyModelTest, SignalRoutingSingleAppendInsideVisiblePage)
{
    set_source_row_count(10);
    m_proxy->set_paging_enabled(true);
    m_proxy->set_page_size(25);
    m_proxy->set_current_page(1);

    const int old_proxy_rows = m_proxy->rowCount();

    QSignalSpy model_reset_spy(m_proxy, &QAbstractItemModel::modelReset);
    QSignalSpy rows_about_to_inserted_spy(m_proxy, &QAbstractItemModel::rowsAboutToBeInserted);
    QSignalSpy rows_inserted_spy(m_proxy, &QAbstractItemModel::rowsInserted);
    QSignalSpy rows_about_to_removed_spy(m_proxy, &QAbstractItemModel::rowsAboutToBeRemoved);
    QSignalSpy rows_removed_spy(m_proxy, &QAbstractItemModel::rowsRemoved);
    QSignalSpy data_changed_spy(m_proxy, &QAbstractItemModel::dataChanged);

    m_source->insertRow(5);  // Insert in the middle of our visible 10 rows

    EXPECT_EQ(m_proxy->rowCount(), old_proxy_rows + 1);

    print_proxy_signal_matrix("single append inside visible", model_reset_spy,
                              rows_about_to_inserted_spy, rows_inserted_spy,
                              rows_about_to_removed_spy, rows_removed_spy, data_changed_spy);

    EXPECT_EQ(model_reset_spy.count(), 0);
    EXPECT_EQ(rows_about_to_inserted_spy.count(), 1);
    EXPECT_EQ(rows_inserted_spy.count(), 1);
}

/**
 * @brief Diagnostic: single remove INSIDE visible page range.
 */
TEST_F(PagingProxyModelTest, SignalRoutingSingleRemoveInsideVisiblePage)
{
    set_source_row_count(10);
    m_proxy->set_paging_enabled(true);
    m_proxy->set_page_size(25);
    m_proxy->set_current_page(1);

    const int old_proxy_rows = m_proxy->rowCount();

    QSignalSpy model_reset_spy(m_proxy, &QAbstractItemModel::modelReset);
    QSignalSpy rows_about_to_inserted_spy(m_proxy, &QAbstractItemModel::rowsAboutToBeInserted);
    QSignalSpy rows_inserted_spy(m_proxy, &QAbstractItemModel::rowsInserted);
    QSignalSpy rows_about_to_removed_spy(m_proxy, &QAbstractItemModel::rowsAboutToBeRemoved);
    QSignalSpy rows_removed_spy(m_proxy, &QAbstractItemModel::rowsRemoved);
    QSignalSpy data_changed_spy(m_proxy, &QAbstractItemModel::dataChanged);

    m_source->removeRow(5);  // Remove from the middle of our visible 10 rows

    EXPECT_EQ(m_proxy->rowCount(), old_proxy_rows - 1);

    print_proxy_signal_matrix("single remove inside visible", model_reset_spy,
                              rows_about_to_inserted_spy, rows_inserted_spy,
                              rows_about_to_removed_spy, rows_removed_spy, data_changed_spy);

    EXPECT_EQ(model_reset_spy.count(), 0);
    EXPECT_EQ(rows_about_to_removed_spy.count(), 1);
    EXPECT_EQ(rows_removed_spy.count(), 1);
}

/**
 * @brief Diagnostic: single dataChanged INSIDE visible page range.
 */
TEST_F(PagingProxyModelTest, SignalRoutingSingleDataChangedInsideVisiblePage)
{
    set_source_row_count(10);
    m_proxy->set_paging_enabled(true);
    m_proxy->set_page_size(25);
    m_proxy->set_current_page(1);

    QSignalSpy model_reset_spy(m_proxy, &QAbstractItemModel::modelReset);
    QSignalSpy rows_about_to_inserted_spy(m_proxy, &QAbstractItemModel::rowsAboutToBeInserted);
    QSignalSpy rows_inserted_spy(m_proxy, &QAbstractItemModel::rowsInserted);
    QSignalSpy rows_about_to_removed_spy(m_proxy, &QAbstractItemModel::rowsAboutToBeRemoved);
    QSignalSpy rows_removed_spy(m_proxy, &QAbstractItemModel::rowsRemoved);
    QSignalSpy data_changed_spy(m_proxy, &QAbstractItemModel::dataChanged);

    const QModelIndex inside_index = m_source->index(5, 0);  // Update middle row
    m_source->setData(inside_index, QString("updated"), Qt::DisplayRole);

    print_proxy_signal_matrix("single dataChanged inside visible", model_reset_spy,
                              rows_about_to_inserted_spy, rows_inserted_spy,
                              rows_about_to_removed_spy, rows_removed_spy, data_changed_spy);

    EXPECT_EQ(model_reset_spy.count(), 0);
    EXPECT_EQ(data_changed_spy.count(), 1);
}

/**
 * @brief Diagnostic: single append outside visible page range.
 */
TEST_F(PagingProxyModelTest, SignalRoutingSingleAppendOutsideVisiblePage)
{
    set_source_row_count(1000);
    m_proxy->set_paging_enabled(true);
    m_proxy->set_page_size(25);
    m_proxy->set_current_page(1);

    const int old_proxy_rows = m_proxy->rowCount();

    QSignalSpy model_reset_spy(m_proxy, &QAbstractItemModel::modelReset);
    QSignalSpy rows_about_to_inserted_spy(m_proxy, &QAbstractItemModel::rowsAboutToBeInserted);
    QSignalSpy rows_inserted_spy(m_proxy, &QAbstractItemModel::rowsInserted);
    QSignalSpy rows_about_to_removed_spy(m_proxy, &QAbstractItemModel::rowsAboutToBeRemoved);
    QSignalSpy rows_removed_spy(m_proxy, &QAbstractItemModel::rowsRemoved);
    QSignalSpy data_changed_spy(m_proxy, &QAbstractItemModel::dataChanged);

    m_source->insertRow(m_source->rowCount());

    EXPECT_EQ(m_proxy->rowCount(), old_proxy_rows);

    print_proxy_signal_matrix("single append outside visible", model_reset_spy,
                              rows_about_to_inserted_spy, rows_inserted_spy,
                              rows_about_to_removed_spy, rows_removed_spy, data_changed_spy);
}

/**
 * @brief Diagnostic: single remove outside visible page range.
 */
TEST_F(PagingProxyModelTest, SignalRoutingSingleRemoveOutsideVisiblePage)
{
    set_source_row_count(1000);
    m_proxy->set_paging_enabled(true);
    m_proxy->set_page_size(25);
    m_proxy->set_current_page(1);

    const int old_proxy_rows = m_proxy->rowCount();

    QSignalSpy model_reset_spy(m_proxy, &QAbstractItemModel::modelReset);
    QSignalSpy rows_about_to_inserted_spy(m_proxy, &QAbstractItemModel::rowsAboutToBeInserted);
    QSignalSpy rows_inserted_spy(m_proxy, &QAbstractItemModel::rowsInserted);
    QSignalSpy rows_about_to_removed_spy(m_proxy, &QAbstractItemModel::rowsAboutToBeRemoved);
    QSignalSpy rows_removed_spy(m_proxy, &QAbstractItemModel::rowsRemoved);
    QSignalSpy data_changed_spy(m_proxy, &QAbstractItemModel::dataChanged);

    m_source->removeRow(m_source->rowCount() - 1);

    EXPECT_EQ(m_proxy->rowCount(), old_proxy_rows);

    print_proxy_signal_matrix("single remove outside visible", model_reset_spy,
                              rows_about_to_inserted_spy, rows_inserted_spy,
                              rows_about_to_removed_spy, rows_removed_spy, data_changed_spy);
}

/**
 * @brief Diagnostic: single dataChanged outside visible page range.
 */
TEST_F(PagingProxyModelTest, SignalRoutingSingleDataChangedOutsideVisiblePage)
{
    set_source_row_count(1000);
    m_proxy->set_paging_enabled(true);
    m_proxy->set_page_size(25);
    m_proxy->set_current_page(1);

    QSignalSpy model_reset_spy(m_proxy, &QAbstractItemModel::modelReset);
    QSignalSpy rows_about_to_inserted_spy(m_proxy, &QAbstractItemModel::rowsAboutToBeInserted);
    QSignalSpy rows_inserted_spy(m_proxy, &QAbstractItemModel::rowsInserted);
    QSignalSpy rows_about_to_removed_spy(m_proxy, &QAbstractItemModel::rowsAboutToBeRemoved);
    QSignalSpy rows_removed_spy(m_proxy, &QAbstractItemModel::rowsRemoved);
    QSignalSpy data_changed_spy(m_proxy, &QAbstractItemModel::dataChanged);

    const QModelIndex far_index = m_source->index(500, 0);
    m_source->setData(far_index, QString("updated"), Qt::DisplayRole);

    EXPECT_EQ(m_proxy->rowCount(), 25);

    print_proxy_signal_matrix("single dataChanged outside visible", model_reset_spy,
                              rows_about_to_inserted_spy, rows_inserted_spy,
                              rows_about_to_removed_spy, rows_removed_spy, data_changed_spy);
}

// --- 5. PERFORMANCE BASELINES ---

/**
 * @brief Baseline benchmark: append rows outside visible page range.
 */
TEST_F(PagingProxyModelTest, AppendRowsOutsideVisiblePagePerformanceBaseline)
{
    constexpr int initial_rows = 20000;
    constexpr int iterations = 2000;

    set_source_row_count(initial_rows);
    m_proxy->set_paging_enabled(true);
    m_proxy->set_page_size(25);
    m_proxy->set_current_page(1);

    QSignalSpy model_reset_spy(m_proxy, &QAbstractItemModel::modelReset);
    QSignalSpy rows_about_to_inserted_spy(m_proxy, &QAbstractItemModel::rowsAboutToBeInserted);
    QSignalSpy rows_inserted_spy(m_proxy, &QAbstractItemModel::rowsInserted);
    QSignalSpy rows_about_to_removed_spy(m_proxy, &QAbstractItemModel::rowsAboutToBeRemoved);
    QSignalSpy rows_removed_spy(m_proxy, &QAbstractItemModel::rowsRemoved);
    QSignalSpy data_changed_spy(m_proxy, &QAbstractItemModel::dataChanged);

    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < iterations; ++i)
    {
        const int append_row = m_source->rowCount();
        m_source->insertRow(append_row);
    }
    auto end = std::chrono::steady_clock::now();

    const auto elapsed_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    const double avg_us_per_op =
        static_cast<double>(elapsed_ms * 1000.0) / static_cast<double>(iterations);

    EXPECT_EQ(m_source->rowCount(), initial_rows + iterations);
    EXPECT_EQ(m_proxy->rowCount(), 25);

    std::cout << "[PERF] PagingProxyModel append-outside-visible baseline: " << elapsed_ms
              << " ms total for " << iterations << " appends (" << avg_us_per_op << " us/op)"
              << std::endl;

    print_proxy_signal_matrix("append outside visible benchmark", model_reset_spy,
                              rows_about_to_inserted_spy, rows_inserted_spy,
                              rows_about_to_removed_spy, rows_removed_spy, data_changed_spy);
}

/**
 * @brief Baseline benchmark: remove rows outside visible page range.
 */
TEST_F(PagingProxyModelTest, RemoveRowsOutsideVisiblePagePerformanceBaseline)
{
    constexpr int initial_rows = 20000;
    constexpr int iterations = 2000;

    set_source_row_count(initial_rows);
    m_proxy->set_paging_enabled(true);
    m_proxy->set_page_size(25);
    m_proxy->set_current_page(1);

    QSignalSpy model_reset_spy(m_proxy, &QAbstractItemModel::modelReset);
    QSignalSpy rows_about_to_inserted_spy(m_proxy, &QAbstractItemModel::rowsAboutToBeInserted);
    QSignalSpy rows_inserted_spy(m_proxy, &QAbstractItemModel::rowsInserted);
    QSignalSpy rows_about_to_removed_spy(m_proxy, &QAbstractItemModel::rowsAboutToBeRemoved);
    QSignalSpy rows_removed_spy(m_proxy, &QAbstractItemModel::rowsRemoved);
    QSignalSpy data_changed_spy(m_proxy, &QAbstractItemModel::dataChanged);

    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < iterations; ++i)
    {
        const int last_row = m_source->rowCount() - 1;
        m_source->removeRow(last_row);
    }
    auto end = std::chrono::steady_clock::now();

    const auto elapsed_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    const double avg_us_per_op =
        static_cast<double>(elapsed_ms * 1000.0) / static_cast<double>(iterations);

    EXPECT_EQ(m_source->rowCount(), initial_rows - iterations);
    EXPECT_EQ(m_proxy->rowCount(), 25);

    std::cout << "[PERF] PagingProxyModel remove-outside-visible baseline: " << elapsed_ms
              << " ms total for " << iterations << " removes (" << avg_us_per_op << " us/op)"
              << std::endl;

    print_proxy_signal_matrix("remove outside visible benchmark", model_reset_spy,
                              rows_about_to_inserted_spy, rows_inserted_spy,
                              rows_about_to_removed_spy, rows_removed_spy, data_changed_spy);
}

/**
 * @brief Baseline benchmark: dataChanged updates outside visible page range.
 */
TEST_F(PagingProxyModelTest, DataChangedOutsideVisiblePagePerformanceBaseline)
{
    constexpr int initial_rows = 20000;
    constexpr int iterations = 3000;
    constexpr int updated_row = 15000;

    set_source_row_count(initial_rows);
    m_proxy->set_paging_enabled(true);
    m_proxy->set_page_size(25);
    m_proxy->set_current_page(1);

    ASSERT_LT(updated_row, m_source->rowCount());

    QSignalSpy model_reset_spy(m_proxy, &QAbstractItemModel::modelReset);
    QSignalSpy rows_about_to_inserted_spy(m_proxy, &QAbstractItemModel::rowsAboutToBeInserted);
    QSignalSpy rows_inserted_spy(m_proxy, &QAbstractItemModel::rowsInserted);
    QSignalSpy rows_about_to_removed_spy(m_proxy, &QAbstractItemModel::rowsAboutToBeRemoved);
    QSignalSpy rows_removed_spy(m_proxy, &QAbstractItemModel::rowsRemoved);
    QSignalSpy data_changed_spy(m_proxy, &QAbstractItemModel::dataChanged);

    const QModelIndex target_index = m_source->index(updated_row, 0);

    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < iterations; ++i)
    {
        m_source->setData(target_index, i, Qt::DisplayRole);
    }
    auto end = std::chrono::steady_clock::now();

    const auto elapsed_ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    const double avg_us_per_op =
        static_cast<double>(elapsed_ms * 1000.0) / static_cast<double>(iterations);

    EXPECT_EQ(m_proxy->rowCount(), 25);

    std::cout << "[PERF] PagingProxyModel dataChanged-outside-visible baseline: " << elapsed_ms
              << " ms total for " << iterations << " updates (" << avg_us_per_op << " us/op)"
              << std::endl;

    print_proxy_signal_matrix("dataChanged outside visible benchmark", model_reset_spy,
                              rows_about_to_inserted_spy, rows_inserted_spy,
                              rows_about_to_removed_spy, rows_removed_spy, data_changed_spy);
}
