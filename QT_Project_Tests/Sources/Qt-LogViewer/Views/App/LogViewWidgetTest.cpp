#include "Qt-LogViewer/Views/App/LogViewWidgetTest.h"

#include <QApplication>
#include <QItemSelectionModel>
#include <QMenu>
#include <QSignalSpy>
#include <QStandardItemModel>
#include <QTemporaryFile>
#include <QToolButton>

#include "Qt-LogViewer/Views/App/LogFilterWidget.h"
#include "Qt-LogViewer/Views/App/LogTableView.h"

/**
 * @brief Constructs the test fixture.
 */
LogViewWidgetTest::LogViewWidgetTest(): m_widget(nullptr) {}

/**
 * @brief Destroys the test fixture.
 */
LogViewWidgetTest::~LogViewWidgetTest() = default;

/**
 * @brief Set up before each test: create widget and show it so visibility-based assertions work.
 */
void LogViewWidgetTest::SetUp()
{
    m_widget = new LogViewWidget();
    m_widget->resize(640, 480);
    // Show the widget so child visibility changes behave predictably in tests
    m_widget->show();
    QApplication::processEvents();
}

/**
 * @brief Tear down after each test.
 */
void LogViewWidgetTest::TearDown()
{
    delete m_widget;
    m_widget = nullptr;
    QApplication::processEvents();
}

/**
 * @brief Locate the files-in-view menu (objectName "filesInViewMenu").
 */
auto LogViewWidgetTest::find_files_menu() const -> QMenu*
{
    auto* result = m_widget->findChild<QMenu*>(QStringLiteral("filesInViewMenu"));
    return result;
}

/**
 * @brief Show the files menu and collect created per-file row widgets.
 */
auto LogViewWidgetTest::show_and_collect_file_rows() -> QVector<FilesInViewMenuItemWidget*>
{
    QVector<FilesInViewMenuItemWidget*> result;

    QMenu* menu = find_files_menu();
    if (menu != nullptr)
    {
        // Show menu to force creation of QWidgetAction widgets
        menu->show();
        QApplication::processEvents();

        const auto rows = menu->findChildren<FilesInViewMenuItemWidget*>();
        for (auto* r: rows)
        {
            result.append(r);
        }
    }

    return result;
}

/**
 * @brief Test view id getter/setter and view_id_changed emission.
 */
TEST_F(LogViewWidgetTest, ViewIdSetGetEmitsWhenChanged)
{
    ASSERT_NE(m_widget, nullptr);

    QSignalSpy spy(m_widget, &LogViewWidget::view_id_changed);
    const QUuid id1 = QUuid::createUuid();
    const QUuid id2 = QUuid::createUuid();

    m_widget->set_view_id(id1);
    EXPECT_EQ(m_widget->get_view_id(), id1);
    EXPECT_EQ(spy.count(), 1);

    // Setting same id should not emit
    m_widget->set_view_id(id1);
    EXPECT_EQ(spy.count(), 1);

    m_widget->set_view_id(id2);
    EXPECT_EQ(m_widget->get_view_id(), id2);
    EXPECT_EQ(spy.count(), 2);
}

/**
 * @brief set_model wires selection model to forward current_row_changed.
 */
TEST_F(LogViewWidgetTest, SetModelWiresSelectionForwarding)
{
    ASSERT_NE(m_widget, nullptr);

    // Build a simple model with 2 rows
    QStandardItemModel model(2, 1);
    model.setData(model.index(0, 0), QStringLiteral("r0"));
    model.setData(model.index(1, 0), QStringLiteral("r1"));

    m_widget->set_model(&model);

    QSignalSpy spy(m_widget, &LogViewWidget::current_row_changed);

    // Select second row via table's selection model
    QItemSelectionModel* sel = m_widget->get_table_view()->selectionModel();
    ASSERT_NE(sel, nullptr);

    sel->setCurrentIndex(model.index(1, 0), QItemSelectionModel::SelectCurrent);
    QApplication::processEvents();

    EXPECT_GE(spy.count(), 1);
}

/**
 * @brief set_app_names and forwarded app_filter_changed signal from filter widget.
 */
TEST_F(LogViewWidgetTest, AppFilterForwardingAndGetters)
{
    ASSERT_NE(m_widget, nullptr);

    QSignalSpy spy_app(m_widget, &LogViewWidget::app_filter_changed);

    // Populate app names via view API and then instruct filter widget to pick one
    QSet<QString> names;
    names.insert(QStringLiteral("AppA"));
    names.insert(QStringLiteral("AppB"));

    m_widget->set_app_names(names);
    // Use filter widget API to select value and expect the LogViewWidget to forward the signal
    LogFilterWidget* filter = m_widget->get_filter_widget();
    ASSERT_NE(filter, nullptr);

    filter->set_current_app_name_filter(QStringLiteral("AppA"));
    QApplication::processEvents();

    EXPECT_GE(spy_app.count(), 1);
    EXPECT_EQ(m_widget->get_current_app_name(), QStringLiteral("AppA"));
}

/**
 * @brief Level filter setters/getters and forwarded level-change signal.
 *
 * Note: Filter/proxy normalizes levels (lowercase). Compare normalized sets.
 */
TEST_F(LogViewWidgetTest, LogLevelFiltersAndCounts)
{
    ASSERT_NE(m_widget, nullptr);

    QSignalSpy spy_levels(m_widget, &LogViewWidget::log_level_filter_changed);

    QVector<QString> avail;
    avail.append(QStringLiteral("INFO"));
    avail.append(QStringLiteral("ERROR"));

    m_widget->set_available_log_levels(avail);

    QSet<QString> levels;
    levels.insert(QStringLiteral("INFO"));
    m_widget->set_log_levels(levels);

    QMap<QString, int> counts;
    counts.insert(QStringLiteral("INFO"), 3);
    counts.insert(QStringLiteral("ERROR"), 1);
    m_widget->set_log_level_counts(counts);

    // Forward changes via the underlying filter widget
    LogFilterWidget* filter = m_widget->get_filter_widget();
    ASSERT_NE(filter, nullptr);

    filter->set_log_levels(levels);
    QApplication::processEvents();

    EXPECT_GE(spy_levels.count(), 1);

    // Expected normalized (lowercase)
    QSet<QString> expected_normalized;
    for (const auto& lv: levels)
    {
        expected_normalized.insert(lv.trimmed().toLower());
    }
    EXPECT_EQ(m_widget->get_current_log_levels(), expected_normalized);
}

/**
 * @brief set_filter_widget_visible and clear_filters should not crash; clear resets levels.
 *
 * App name may remain unchanged after clearing source lists; ensure it is not forcibly "AppA".
 */
TEST_F(LogViewWidgetTest, FilterVisibilityAndClearFilters)
{
    ASSERT_NE(m_widget, nullptr);

    // Ensure some initial app names + selection exist
    QSet<QString> names;
    names.insert(QStringLiteral("AppA"));
    names.insert(QStringLiteral("AppB"));
    m_widget->set_app_names(names);
    LogFilterWidget* filter = m_widget->get_filter_widget();
    ASSERT_NE(filter, nullptr);
    filter->set_current_app_name_filter(QStringLiteral("AppA"));
    QApplication::processEvents();

    // Hide filter widget
    m_widget->set_filter_widget_visible(false);
    EXPECT_FALSE(m_widget->get_filter_widget()->isVisible());

    // Show filter widget
    m_widget->set_filter_widget_visible(true);
    EXPECT_TRUE(m_widget->get_filter_widget()->isVisible());

    // Clear filters should reset level selection; app names list becomes empty,
    // but current app selection may remain non-empty depending on widget behavior.
    m_widget->clear_filters();
    EXPECT_TRUE(m_widget->get_filter_widget()->get_current_log_levels().isEmpty());

    // After clearing, attempting to select a non-existent app should not result in "AppA"
    QSignalSpy spy_app(m_widget, &LogViewWidget::app_filter_changed);
    filter->set_current_app_name_filter(QStringLiteral("AppA"));
    QApplication::processEvents();
    EXPECT_EQ(spy_app.count(), 0);
    EXPECT_NE(m_widget->get_current_app_name(), QStringLiteral("AppA"));
}

/**
 * @brief set_view_file_paths rebuilds menu and forwards per-file widget actions.
 */
TEST_F(LogViewWidgetTest, SetViewFilePathsBuildsMenuAndForwardsActions)
{
    ASSERT_NE(m_widget, nullptr);

    // Prepare two temporary files to act as file paths
    QTemporaryFile f1;
    QTemporaryFile f2;
    ASSERT_TRUE(f1.open());
    ASSERT_TRUE(f2.open());
    f1.close();
    f2.close();

    QVector<QString> paths;
    paths.append(f1.fileName());
    paths.append(f2.fileName());

    m_widget->set_view_file_paths(paths);

    QMenu* menu = find_files_menu();
    ASSERT_NE(menu, nullptr);

    // Show menu to create per-file widgets
    menu->show();
    QApplication::processEvents();

    // Collect row widgets created inside the menu
    const auto rows = menu->findChildren<FilesInViewMenuItemWidget*>();
    EXPECT_EQ(rows.size(), 2);

    // Wire spies on LogViewWidget signals
    QSignalSpy spy_show_only(m_widget, &LogViewWidget::show_only_file_requested);
    QSignalSpy spy_toggle(m_widget, &LogViewWidget::toggle_visibility_requested);
    QSignalSpy spy_remove(m_widget, &LogViewWidget::remove_file_requested);

    // Trigger actions by clicking embedded buttons
    for (auto* row: rows)
    {
        ASSERT_NE(row, nullptr);
        // find inline buttons
        QToolButton* btn_show = nullptr;
        QToolButton* btn_toggle = nullptr;
        QToolButton* btn_remove = nullptr;

        const auto buttons = row->findChildren<QToolButton*>();
        for (const auto* b: buttons)
        {
            const QString tt = b->toolTip();
            if (tt.contains(QStringLiteral("Show only"), Qt::CaseInsensitive))
            {
                btn_show = const_cast<QToolButton*>(b);
            }
            else if (tt.contains(QStringLiteral("Hide"), Qt::CaseInsensitive) ||
                     tt.contains(QStringLiteral("Show this file"), Qt::CaseInsensitive))
            {
                btn_toggle = const_cast<QToolButton*>(b);
            }
            else
            {
                btn_remove = const_cast<QToolButton*>(b);
            }
        }

        ASSERT_NE(btn_show, nullptr);
        ASSERT_NE(btn_toggle, nullptr);
        ASSERT_NE(btn_remove, nullptr);

        // Click each and expect the LogViewWidget to forward
        btn_show->click();
        btn_toggle->click();
        btn_remove->click();
    }

    // We expect at least one emission per row per action
    EXPECT_GE(spy_show_only.count(), rows.size());
    EXPECT_GE(spy_toggle.count(), rows.size());
    EXPECT_GE(spy_remove.count(), rows.size());

    // Close menu and allow cleanup
    menu->close();
    QApplication::processEvents();
}
