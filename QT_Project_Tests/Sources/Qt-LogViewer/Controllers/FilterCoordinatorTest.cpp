#include "Qt-LogViewer/Controllers/FilterCoordinatorTest.h"

#include <QList>
#include <QMap>
#include <QTextStream>
#include <QVector>

/**
 * @brief Constructs the test fixture.
 */
FilterCoordinatorTest::FilterCoordinatorTest(): m_views(nullptr), m_fc(nullptr), m_view() {}

/**
 * @brief Destroys the test fixture.
 */
FilterCoordinatorTest::~FilterCoordinatorTest() = default;

/**
 * @brief Set up before each test: create ViewRegistry, a view, and FilterCoordinator.
 */
void FilterCoordinatorTest::SetUp()
{
    m_views = new ViewRegistry();
    m_view = m_views->create_view();
    m_fc = new FilterCoordinator(m_views);
}

/**
 * @brief Tear down after each test.
 */
void FilterCoordinatorTest::TearDown()
{
    delete m_fc;
    m_fc = nullptr;

    delete m_views;
    m_views = nullptr;
}

/**
 * @brief Creates a unique temp file path (does not create the file).
 *
 * @param stem Stem to include in the file name for identification.
 */
auto FilterCoordinatorTest::make_temp_abs_path(const QString& stem) const -> QString
{
    const QString base = QDir::tempPath();
    const QString name = QStringLiteral("qt_lv_filter_%1_%2.log")
                             .arg(stem, QUuid::createUuid().toString(QUuid::WithoutBraces));
    return QDir(base).filePath(name);
}

/**
 * @brief Helper to get the proxy model for a view id.
 *
 * @param view_id Target view id.
 * @return Pointer to LogSortFilterProxyModel if found; nullptr otherwise.
 */
auto FilterCoordinatorTest::proxy_for(const QUuid& view_id) const -> LogSortFilterProxyModel*
{
    auto* ctx = m_views->get_context(view_id);
    return ctx != nullptr ? ctx->get_sort_proxy() : nullptr;
}

/**
 * @brief App-name setter delegates to proxy and getters return same value.
 */
TEST_F(FilterCoordinatorTest, SetAppNameAndGettersRoundtrip)
{
    ASSERT_NE(m_fc, nullptr);

    const QString app = QStringLiteral("MyApp");
    m_fc->set_app_name(m_view, app);

    auto* proxy = proxy_for(m_view);
    ASSERT_NE(proxy, nullptr);

    EXPECT_EQ(proxy->get_app_name_filter(), app);
    EXPECT_EQ(m_fc->get_app_name(m_view), app);
}

/**
 * @brief Log level filters delegate to proxy and getters return same set (case-insensitive).
 *
 * Note: LogSortFilterProxyModel normalizes levels to lowercase internally, so compare
 * case-insensitively.
 */
TEST_F(FilterCoordinatorTest, SetLogLevelsAndGettersRoundtrip)
{
    ASSERT_NE(m_fc, nullptr);

    QSet<QString> levels{QStringLiteral("Info"), QStringLiteral("Error"), QStringLiteral("Debug")};
    m_fc->set_log_levels(m_view, levels);

    auto* proxy = proxy_for(m_view);
    ASSERT_NE(proxy, nullptr);

    auto to_lower = [](const QSet<QString>& s) {
        QSet<QString> out;
        for (const auto& v: s)
        {
            out.insert(v.toLower());
        }
        return out;
    };

    EXPECT_EQ(to_lower(proxy->get_log_level_filters()), to_lower(levels));
    EXPECT_EQ(to_lower(m_fc->get_log_levels(m_view)), to_lower(levels));
}

/**
 * @brief Search filter delegates to proxy and getters reflect text/field/regex.
 */
TEST_F(FilterCoordinatorTest, SetSearchAndGettersRoundtrip)
{
    ASSERT_NE(m_fc, nullptr);

    const QString text = QStringLiteral("needle");
    const QString field = QStringLiteral("message");
    const bool use_regex = true;

    m_fc->set_search(m_view, text, field, use_regex);

    auto* proxy = proxy_for(m_view);
    ASSERT_NE(proxy, nullptr);

    EXPECT_EQ(proxy->get_search_text(), text);
    EXPECT_EQ(proxy->get_search_field(), field);
    EXPECT_TRUE(proxy->is_search_regex());

    EXPECT_EQ(m_fc->get_search_text(m_view), text);
    EXPECT_EQ(m_fc->get_search_field(m_view), field);
    EXPECT_TRUE(m_fc->is_search_regex(m_view));
}

/**
 * @brief set_show_only applies path, unhides target, and reset clears hidden set.
 */
TEST_F(FilterCoordinatorTest, ShowOnlyApplyAndReset)
{
    ASSERT_NE(m_fc, nullptr);

    const QString f1 = make_temp_abs_path("a");
    const QString f2 = make_temp_abs_path("b");

    // Seed loaded files and hidden one of them
    QList<LogFileInfo> files{LogFileInfo(f1), LogFileInfo(f2)};
    m_views->set_loaded_files(m_view, files);

    auto* proxy = proxy_for(m_view);
    ASSERT_NE(proxy, nullptr);

    proxy->hide_file(f1);
    ASSERT_TRUE(proxy->get_hidden_file_paths().contains(f1));

    // Apply show-only -> set, and target must be unhidden
    m_fc->set_show_only(m_view, f1);
    EXPECT_EQ(proxy->get_show_only_file_path(), f1);
    EXPECT_FALSE(proxy->get_hidden_file_paths().contains(f1));

    // Reset show-only -> empty and hidden cleared
    m_fc->set_show_only(m_view, QString());
    EXPECT_TRUE(proxy->get_show_only_file_path().isEmpty());
    EXPECT_TRUE(proxy->get_hidden_file_paths().isEmpty());
}

/**
 * @brief toggle_visibility without show-only toggles hidden/unhidden for the file.
 */
TEST_F(FilterCoordinatorTest, ToggleVisibilityWithoutShowOnly)
{
    ASSERT_NE(m_fc, nullptr);

    const QString f1 = make_temp_abs_path("t1");
    const QString f2 = make_temp_abs_path("t2");
    m_views->set_loaded_files(m_view, QList<LogFileInfo>{LogFileInfo(f1), LogFileInfo(f2)});

    auto* proxy = proxy_for(m_view);
    ASSERT_NE(proxy, nullptr);

    // No show-only active
    EXPECT_TRUE(proxy->get_show_only_file_path().isEmpty());

    // Toggle f1 -> hidden
    m_fc->toggle_visibility(m_view, f1);
    EXPECT_TRUE(proxy->get_hidden_file_paths().contains(f1));

    // Toggle f1 again -> unhidden
    m_fc->toggle_visibility(m_view, f1);
    EXPECT_FALSE(proxy->get_hidden_file_paths().contains(f1));
}

/**
 * @brief toggle_visibility on current show-only target clears show-only and hides all remaining
 * files.
 */
TEST_F(FilterCoordinatorTest, ToggleVisibilityShowOnlyOnTargetHidesAll)
{
    ASSERT_NE(m_fc, nullptr);

    const QString f1 = make_temp_abs_path("s1");
    const QString f2 = make_temp_abs_path("s2");
    m_views->set_loaded_files(m_view, QList<LogFileInfo>{LogFileInfo(f1), LogFileInfo(f2)});

    auto* proxy = proxy_for(m_view);
    ASSERT_NE(proxy, nullptr);

    proxy->set_show_only_file_path(f1);
    ASSERT_EQ(proxy->get_show_only_file_path(), f1);

    m_fc->toggle_visibility(m_view, f1);

    EXPECT_TRUE(proxy->get_show_only_file_path().isEmpty());

    const QSet<QString> hidden = proxy->get_hidden_file_paths();
    // All files in view should be hidden now
    EXPECT_TRUE(hidden.contains(f1));
    EXPECT_TRUE(hidden.contains(f2));
}

/**
 * @brief toggle_visibility on non-target while show-only active clears show-only and hides others.
 */
TEST_F(FilterCoordinatorTest, ToggleVisibilityShowOnlyOnOtherKeepsTwoVisible)
{
    ASSERT_NE(m_fc, nullptr);

    const QString f1 = make_temp_abs_path("x1");
    const QString f2 = make_temp_abs_path("x2");
    const QString f3 = make_temp_abs_path("x3");
    m_views->set_loaded_files(
        m_view, QList<LogFileInfo>{LogFileInfo(f1), LogFileInfo(f2), LogFileInfo(f3)});

    auto* proxy = proxy_for(m_view);
    ASSERT_NE(proxy, nullptr);

    proxy->set_show_only_file_path(f1);
    // Pre-hide f3 to ensure pre-existing hidden survive (minus exceptions)
    proxy->hide_file(f3);
    ASSERT_TRUE(proxy->get_hidden_file_paths().contains(f3));

    m_fc->toggle_visibility(m_view, f2);

    EXPECT_TRUE(proxy->get_show_only_file_path().isEmpty());
    // After toggle, only f3 should remain hidden
    const QSet<QString> hidden_after = proxy->get_hidden_file_paths();
    EXPECT_FALSE(hidden_after.contains(f1));
    EXPECT_FALSE(hidden_after.contains(f2));
    EXPECT_TRUE(hidden_after.contains(f3));
}

/**
 * @brief hide_file delegates to proxy.
 */
TEST_F(FilterCoordinatorTest, HideFileDelegates)
{
    ASSERT_NE(m_fc, nullptr);

    const QString f1 = make_temp_abs_path("h1");
    m_views->set_loaded_files(m_view, QList<LogFileInfo>{LogFileInfo(f1)});

    auto* proxy = proxy_for(m_view);
    ASSERT_NE(proxy, nullptr);

    m_fc->hide_file(m_view, f1);
    EXPECT_TRUE(proxy->get_hidden_file_paths().contains(f1));
}

/**
 * @brief get_log_level_counts returns counts built from ViewRegistry entries.
 */
TEST_F(FilterCoordinatorTest, GetLogLevelCountsFromEntries)
{
    ASSERT_NE(m_fc, nullptr);

    auto* ctx = m_views->get_context(m_view);
    ASSERT_NE(ctx, nullptr);

    QVector<LogEntry> entries;
    entries.append(
        LogEntry(QDateTime(), QStringLiteral("Info"), QStringLiteral("a"), LogFileInfo()));
    entries.append(
        LogEntry(QDateTime(), QStringLiteral("Error"), QStringLiteral("b"), LogFileInfo()));
    entries.append(
        LogEntry(QDateTime(), QStringLiteral("Info"), QStringLiteral("c"), LogFileInfo()));
    ctx->append_entries(entries);

    const QMap<QString, int> counts = m_fc->get_log_level_counts(m_view);
    EXPECT_EQ(counts.value(QStringLiteral("Info")), 2);
    EXPECT_EQ(counts.value(QStringLiteral("Error")), 1);
    EXPECT_EQ(counts.value(QStringLiteral("Debug")), 0);
}

/**
 * @brief get_available_log_levels returns the canonical set of levels.
 */
TEST_F(FilterCoordinatorTest, GetAvailableLogLevelsStaticOrder)
{
    const QVector<QString> levels = FilterCoordinator::get_available_log_levels();
    ASSERT_EQ(levels.size(), 6);
    EXPECT_EQ(levels[0], QStringLiteral("Trace"));
    EXPECT_EQ(levels[1], QStringLiteral("Debug"));
    EXPECT_EQ(levels[2], QStringLiteral("Info"));
    EXPECT_EQ(levels[3], QStringLiteral("Warning"));
    EXPECT_EQ(levels[4], QStringLiteral("Error"));
    EXPECT_EQ(levels[5], QStringLiteral("Fatal"));
}

/**
 * @brief adjust_visibility_on_file_removed clears show-only and hides remaining if target removed.
 */
TEST_F(FilterCoordinatorTest, AdjustVisibilityOnFileRemovedClearsShowOnlyAndHideRemaining)
{
    ASSERT_NE(m_fc, nullptr);

    const QString f1 = make_temp_abs_path("rm1");
    const QString f2 = make_temp_abs_path("rm2");

    m_views->set_loaded_files(m_view, QList<LogFileInfo>{LogFileInfo(f1), LogFileInfo(f2)});

    auto* proxy = proxy_for(m_view);
    ASSERT_NE(proxy, nullptr);

    proxy->set_show_only_file_path(f1);

    // Simulate file removal from registry state first
    m_views->remove_entries_by_file(m_view, f1);

    m_fc->adjust_visibility_on_file_removed(m_view, f1);

    EXPECT_TRUE(proxy->get_show_only_file_path().isEmpty());

    // Remaining should all be hidden (only f2 should remain in registry)
    const QSet<QString> hidden = proxy->get_hidden_file_paths();
    EXPECT_FALSE(hidden.contains(f1));
    EXPECT_TRUE(hidden.contains(f2));
}

/**
 * @brief adjust_visibility_on_file_removed removes file from hidden set if it was hidden.
 */
TEST_F(FilterCoordinatorTest, AdjustVisibilityOnFileRemovedPrunesHiddenSet)
{
    ASSERT_NE(m_fc, nullptr);

    const QString f1 = make_temp_abs_path("pr1");
    const QString f2 = make_temp_abs_path("pr2");

    m_views->set_loaded_files(m_view, QList<LogFileInfo>{LogFileInfo(f1), LogFileInfo(f2)});

    auto* proxy = proxy_for(m_view);
    ASSERT_NE(proxy, nullptr);

    proxy->hide_file(f1);
    proxy->hide_file(f2);
    ASSERT_TRUE(proxy->get_hidden_file_paths().contains(f1));
    ASSERT_TRUE(proxy->get_hidden_file_paths().contains(f2));

    // Simulate removal of f1 from registry state
    m_views->remove_entries_by_file(m_view, f1);

    m_fc->adjust_visibility_on_file_removed(m_view, f1);

    const QSet<QString> hidden = proxy->get_hidden_file_paths();
    EXPECT_FALSE(hidden.contains(f1));
    EXPECT_TRUE(hidden.contains(f2));
}

/**
 * @brief adjust_visibility_on_global_file_removed applies per-view logic across all views.
 */
TEST_F(FilterCoordinatorTest, AdjustVisibilityOnGlobalFileRemovedUpdatesAllViews)
{
    ASSERT_NE(m_fc, nullptr);

    // Create a second view
    const QUuid v2 = m_views->create_view();

    const QString f1 = make_temp_abs_path("g1");
    const QString f2 = make_temp_abs_path("g2");
    const QString f3 = make_temp_abs_path("g3");

    // View A: show-only on f1, files [f1, f2]
    m_views->set_loaded_files(m_view, QList<LogFileInfo>{LogFileInfo(f1), LogFileInfo(f2)});
    auto* pA = proxy_for(m_view);
    ASSERT_NE(pA, nullptr);
    pA->set_show_only_file_path(f1);

    // View B: hidden includes f1 and f3, files [f1, f3]
    m_views->set_loaded_files(v2, QList<LogFileInfo>{LogFileInfo(f1), LogFileInfo(f3)});
    auto* pB = proxy_for(v2);
    ASSERT_NE(pB, nullptr);
    pB->hide_file(f1);
    pB->hide_file(f3);

    // Simulate file removal in registry for both views
    m_views->remove_entries_by_file(m_view, f1);
    m_views->remove_entries_by_file(v2, f1);

    // Apply global adjust
    m_fc->adjust_visibility_on_global_file_removed(f1);

    // View A: show-only cleared, remaining (f2) hidden
    EXPECT_TRUE(pA->get_show_only_file_path().isEmpty());
    EXPECT_TRUE(pA->get_hidden_file_paths().contains(f2));
    EXPECT_FALSE(pA->get_hidden_file_paths().contains(f1));

    // View B: removed file pruned from hidden, f3 remains hidden
    const QSet<QString> hiddenB = pB->get_hidden_file_paths();
    EXPECT_FALSE(hiddenB.contains(f1));
    EXPECT_TRUE(hiddenB.contains(f3));
}
