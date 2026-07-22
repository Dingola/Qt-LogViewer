#include "Qt-LogViewer/Controllers/FilterCoordinatorTest.h"

#include <QDir>
#include <QList>
#include <QMap>
#include <QSet>
#include <QVector>

#include "Qt-LogViewer/Controllers/LogViewContext.h"
#include "Qt-LogViewer/Models/LogEntry.h"

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
 * @brief Returns the filter state of a view.
 * @param view_id Target view id.
 * @return Current filter state or a default state if the view does not exist.
 */
auto FilterCoordinatorTest::filter_state_for(const QUuid& view_id) const -> FilterState
{
    const LogViewContext* context = m_views->get_context(view_id);
    const FilterState state = context != nullptr ? context->get_filter_state() : FilterState();

    return state;
}

/**
 * @brief App-name setter updates the view state and coordinator getter.
 */
TEST_F(FilterCoordinatorTest, SetAppNameAndGettersRoundtrip)
{
    ASSERT_NE(m_fc, nullptr);

    const QString app = QStringLiteral("MyApp");
    m_fc->set_app_name(m_view, app);

    const FilterState state = filter_state_for(m_view);

    EXPECT_EQ(state.app_name, app);
    EXPECT_EQ(m_fc->get_app_name(m_view), app);
}

/**
 * @brief Log-level filters are normalized and stored in the view state.
 */
TEST_F(FilterCoordinatorTest, SetLogLevelsAndGettersRoundtrip)
{
    ASSERT_NE(m_fc, nullptr);

    const QSet<QString> levels{QStringLiteral("Info"), QStringLiteral(" Error "),
                               QStringLiteral("DEBUG")};

    const QSet<QString> expected{QStringLiteral("info"), QStringLiteral("error"),
                                 QStringLiteral("debug")};

    m_fc->set_log_levels(m_view, levels);

    const FilterState state = filter_state_for(m_view);

    EXPECT_EQ(state.log_levels, expected);
    EXPECT_EQ(m_fc->get_log_levels(m_view), expected);
}

/**
 * @brief Search settings are stored in the view state and returned by the coordinator.
 */
TEST_F(FilterCoordinatorTest, SetSearchAndGettersRoundtrip)
{
    ASSERT_NE(m_fc, nullptr);

    const QString text = QStringLiteral("needle");
    const SearchField field = SearchField::Message;
    const bool use_regex = true;

    m_fc->set_search(m_view, text, field, use_regex);

    const FilterState state = filter_state_for(m_view);

    EXPECT_EQ(state.search_text, text);
    EXPECT_EQ(state.search_field, field);
    EXPECT_TRUE(state.use_regex);

    EXPECT_EQ(m_fc->get_search_text(m_view), text);
    EXPECT_EQ(m_fc->get_search_field(m_view), field);
    EXPECT_TRUE(m_fc->is_search_regex(m_view));
}

/**
 * @brief Show-only unhides its target and resetting it clears hidden files.
 */
TEST_F(FilterCoordinatorTest, ShowOnlyApplyAndReset)
{
    ASSERT_NE(m_fc, nullptr);

    const QString f1 = make_temp_abs_path("a");
    const QString f2 = make_temp_abs_path("b");

    m_views->set_loaded_files(m_view, QList<LogFileInfo>{LogFileInfo(f1), LogFileInfo(f2)});

    m_fc->hide_file(m_view, f1);

    FilterState state = filter_state_for(m_view);
    ASSERT_TRUE(state.hidden_files.contains(f1));

    m_fc->set_show_only(m_view, f1);

    state = filter_state_for(m_view);
    EXPECT_EQ(state.show_only_file, f1);
    EXPECT_FALSE(state.hidden_files.contains(f1));

    m_fc->set_show_only(m_view, QString());

    state = filter_state_for(m_view);
    EXPECT_TRUE(state.show_only_file.isEmpty());
    EXPECT_TRUE(state.hidden_files.isEmpty());
}

/**
 * @brief File visibility can be toggled without an active show-only filter.
 */
TEST_F(FilterCoordinatorTest, ToggleVisibilityWithoutShowOnly)
{
    ASSERT_NE(m_fc, nullptr);

    const QString f1 = make_temp_abs_path("t1");
    const QString f2 = make_temp_abs_path("t2");

    m_views->set_loaded_files(m_view, QList<LogFileInfo>{LogFileInfo(f1), LogFileInfo(f2)});

    FilterState state = filter_state_for(m_view);
    EXPECT_TRUE(state.show_only_file.isEmpty());

    m_fc->toggle_visibility(m_view, f1);

    state = filter_state_for(m_view);
    EXPECT_TRUE(state.hidden_files.contains(f1));

    m_fc->toggle_visibility(m_view, f1);

    state = filter_state_for(m_view);
    EXPECT_FALSE(state.hidden_files.contains(f1));
}

/**
 * @brief Toggling the show-only target clears show-only and hides every file.
 */
TEST_F(FilterCoordinatorTest, ToggleVisibilityShowOnlyOnTargetHidesAll)
{
    ASSERT_NE(m_fc, nullptr);

    const QString f1 = make_temp_abs_path("s1");
    const QString f2 = make_temp_abs_path("s2");

    m_views->set_loaded_files(m_view, QList<LogFileInfo>{LogFileInfo(f1), LogFileInfo(f2)});

    m_fc->set_show_only(m_view, f1);

    FilterState state = filter_state_for(m_view);
    ASSERT_EQ(state.show_only_file, f1);

    m_fc->toggle_visibility(m_view, f1);

    state = filter_state_for(m_view);

    EXPECT_TRUE(state.show_only_file.isEmpty());
    EXPECT_TRUE(state.hidden_files.contains(f1));
    EXPECT_TRUE(state.hidden_files.contains(f2));
}

/**
 * @brief Toggling another file clears show-only and keeps both selected files visible.
 */
TEST_F(FilterCoordinatorTest, ToggleVisibilityShowOnlyOnOtherKeepsTwoVisible)
{
    ASSERT_NE(m_fc, nullptr);

    const QString f1 = make_temp_abs_path("x1");
    const QString f2 = make_temp_abs_path("x2");
    const QString f3 = make_temp_abs_path("x3");

    m_views->set_loaded_files(
        m_view, QList<LogFileInfo>{LogFileInfo(f1), LogFileInfo(f2), LogFileInfo(f3)});

    m_fc->set_show_only(m_view, f1);
    m_fc->hide_file(m_view, f3);

    FilterState state = filter_state_for(m_view);
    ASSERT_EQ(state.show_only_file, f1);
    ASSERT_TRUE(state.hidden_files.contains(f3));

    m_fc->toggle_visibility(m_view, f2);

    state = filter_state_for(m_view);

    EXPECT_TRUE(state.show_only_file.isEmpty());
    EXPECT_FALSE(state.hidden_files.contains(f1));
    EXPECT_FALSE(state.hidden_files.contains(f2));
    EXPECT_TRUE(state.hidden_files.contains(f3));
}

/**
 * @brief Hiding a file stores its path in the view filter state.
 */
TEST_F(FilterCoordinatorTest, HideFileUpdatesViewState)
{
    ASSERT_NE(m_fc, nullptr);

    const QString f1 = make_temp_abs_path("h1");

    m_views->set_loaded_files(m_view, QList<LogFileInfo>{LogFileInfo(f1)});

    m_fc->hide_file(m_view, f1);

    const FilterState state = filter_state_for(m_view);
    EXPECT_TRUE(state.hidden_files.contains(f1));
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
 * @brief Removing the show-only target clears it and hides every remaining file.
 */
TEST_F(FilterCoordinatorTest, AdjustVisibilityOnFileRemovedClearsShowOnlyAndHideRemaining)
{
    ASSERT_NE(m_fc, nullptr);

    const QString f1 = make_temp_abs_path("rm1");
    const QString f2 = make_temp_abs_path("rm2");

    m_views->set_loaded_files(m_view, QList<LogFileInfo>{LogFileInfo(f1), LogFileInfo(f2)});

    m_fc->set_show_only(m_view, f1);

    m_views->remove_entries_by_file(m_view, f1);
    m_fc->adjust_visibility_on_file_removed(m_view, f1);

    const FilterState state = filter_state_for(m_view);

    EXPECT_TRUE(state.show_only_file.isEmpty());
    EXPECT_FALSE(state.hidden_files.contains(f1));
    EXPECT_TRUE(state.hidden_files.contains(f2));
}

/**
 * @brief Removing a hidden file removes its path from the filter state.
 */
TEST_F(FilterCoordinatorTest, AdjustVisibilityOnFileRemovedPrunesHiddenSet)
{
    ASSERT_NE(m_fc, nullptr);

    const QString f1 = make_temp_abs_path("pr1");
    const QString f2 = make_temp_abs_path("pr2");

    m_views->set_loaded_files(m_view, QList<LogFileInfo>{LogFileInfo(f1), LogFileInfo(f2)});

    m_fc->hide_file(m_view, f1);
    m_fc->hide_file(m_view, f2);

    FilterState state = filter_state_for(m_view);
    ASSERT_TRUE(state.hidden_files.contains(f1));
    ASSERT_TRUE(state.hidden_files.contains(f2));

    m_views->remove_entries_by_file(m_view, f1);
    m_fc->adjust_visibility_on_file_removed(m_view, f1);

    state = filter_state_for(m_view);

    EXPECT_FALSE(state.hidden_files.contains(f1));
    EXPECT_TRUE(state.hidden_files.contains(f2));
}

/**
 * @brief Global file removal updates visibility state in every affected view.
 */
TEST_F(FilterCoordinatorTest, AdjustVisibilityOnGlobalFileRemovedUpdatesAllViews)
{
    ASSERT_NE(m_fc, nullptr);

    const QUuid v2 = m_views->create_view();

    const QString f1 = make_temp_abs_path("g1");
    const QString f2 = make_temp_abs_path("g2");
    const QString f3 = make_temp_abs_path("g3");

    m_views->set_loaded_files(m_view, QList<LogFileInfo>{LogFileInfo(f1), LogFileInfo(f2)});
    m_fc->set_show_only(m_view, f1);

    m_views->set_loaded_files(v2, QList<LogFileInfo>{LogFileInfo(f1), LogFileInfo(f3)});
    m_fc->hide_file(v2, f1);
    m_fc->hide_file(v2, f3);

    m_views->remove_entries_by_file(m_view, f1);
    m_views->remove_entries_by_file(v2, f1);

    m_fc->adjust_visibility_on_global_file_removed(f1);

    const FilterState first_state = filter_state_for(m_view);
    EXPECT_TRUE(first_state.show_only_file.isEmpty());
    EXPECT_TRUE(first_state.hidden_files.contains(f2));
    EXPECT_FALSE(first_state.hidden_files.contains(f1));

    const FilterState second_state = filter_state_for(v2);
    EXPECT_FALSE(second_state.hidden_files.contains(f1));
    EXPECT_TRUE(second_state.hidden_files.contains(f3));
}
