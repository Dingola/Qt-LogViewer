#include <gtest/gtest.h>

#include <QFileInfo>
#include <QJsonArray>
#include <QJsonObject>

#include "Qt-LogViewer/Services/SessionCodec.h"

/**
 * @file SessionCodecTest.cpp
 * @brief Verifies session JSON roundtrips, compatibility defaults, and stable output.
 */

namespace
{
/**
 * @brief Verifies that every session field survives serialization and deserialization.
 */
TEST(SessionCodecTest, RoundTripsCompleteSessionState)
{
    const QString file_path = QStringLiteral("logs/application.log");
    const QString absolute_file_path = QFileInfo(file_path).absoluteFilePath();
    const QUuid view_id = QUuid::createUuid();
    const QUuid profile_id = QUuid::createUuid();

    SessionViewState view;
    view.id = view_id;
    view.loaded_files.append(LogFileInfo(file_path, QStringLiteral("Application")));
    view.file_parsing_profile_ids.insert(absolute_file_path, profile_id);
    view.filters.app_name = QStringLiteral("Application");
    view.filters.log_levels = {QStringLiteral("WARN"), QStringLiteral("INFO")};
    view.filters.search_text = QStringLiteral("request failed");
    view.filters.search_field = SearchField::Message;
    view.filters.use_regex = true;
    view.filters.show_only_file = absolute_file_path;
    view.filters.hidden_files = {QStringLiteral("z.log"), QStringLiteral("a.log")};
    view.filters.live_tailing_enabled = false;
    view.page_size = 25;
    view.current_page = 3;
    view.sort_column = 2;
    view.sort_order = Qt::DescendingOrder;
    view.tab_title = QStringLiteral("Application log");

    SessionState state;
    state.id = QStringLiteral("session-id");
    state.name = QStringLiteral("Application session");
    state.views.append(view);
    state.explorer_files.append(
        LogFileInfo(QStringLiteral("logs/other.log"), QStringLiteral("Other")));

    const QJsonObject json = SessionCodec::to_json(state);

    EXPECT_EQ(json.value(QStringLiteral("schema_version")).toInt(),
              SessionCodec::get_schema_version());
    const QJsonObject view_json = json.value(QStringLiteral("views")).toArray().first().toObject();
    EXPECT_EQ(view_json.value(QStringLiteral("current_page")).toInt(), 2);
    EXPECT_EQ(view_json.value(QStringLiteral("sort_order")).toString(), QStringLiteral("desc"));

    const std::optional<SessionState> restored = SessionCodec::from_json(json);

    ASSERT_TRUE(restored.has_value());
    EXPECT_EQ(restored->id, state.id);
    EXPECT_EQ(restored->name, state.name);
    ASSERT_EQ(restored->views.size(), 1);
    ASSERT_EQ(restored->explorer_files.size(), 1);

    const SessionViewState& restored_view = restored->views.first();
    EXPECT_EQ(restored_view.id, view_id);
    ASSERT_EQ(restored_view.loaded_files.size(), 1);
    EXPECT_EQ(restored_view.loaded_files.first().get_file_path(), file_path);
    EXPECT_EQ(restored_view.loaded_files.first().get_app_name(), QStringLiteral("Application"));
    EXPECT_EQ(restored_view.file_parsing_profile_ids.value(absolute_file_path), profile_id);
    EXPECT_EQ(restored_view.filters.app_name, view.filters.app_name);
    EXPECT_EQ(restored_view.filters.log_levels, view.filters.log_levels);
    EXPECT_EQ(restored_view.filters.search_text, view.filters.search_text);
    EXPECT_EQ(restored_view.filters.search_field, view.filters.search_field);
    EXPECT_EQ(restored_view.filters.use_regex, view.filters.use_regex);
    EXPECT_EQ(restored_view.filters.show_only_file, view.filters.show_only_file);
    EXPECT_EQ(restored_view.filters.hidden_files, view.filters.hidden_files);
    EXPECT_EQ(restored_view.filters.live_tailing_enabled, view.filters.live_tailing_enabled);
    EXPECT_EQ(restored_view.page_size, view.page_size);
    EXPECT_EQ(restored_view.current_page, view.current_page);
    EXPECT_EQ(restored_view.sort_column, view.sort_column);
    EXPECT_EQ(restored_view.sort_order, view.sort_order);
    EXPECT_EQ(restored_view.tab_title, view.tab_title);
    EXPECT_EQ(restored->explorer_files.first().get_file_path(), QStringLiteral("logs/other.log"));
}

/**
 * @brief Verifies defaults used when older session documents omit newer fields.
 */
TEST(SessionCodecTest, AppliesDefaultsToOlderDocuments)
{
    QJsonObject view;
    view.insert(QStringLiteral("id"), QUuid::createUuid().toString(QUuid::WithoutBraces));

    QJsonObject json;
    json.insert(QStringLiteral("name"), QStringLiteral("Legacy session"));
    json.insert(QStringLiteral("views"), QJsonArray{view});

    const std::optional<SessionState> restored =
        SessionCodec::from_json(json, QStringLiteral("legacy-id"));

    ASSERT_TRUE(restored.has_value());
    EXPECT_EQ(restored->schema_version, 1);
    EXPECT_EQ(restored->id, QStringLiteral("legacy-id"));
    ASSERT_EQ(restored->views.size(), 1);
    EXPECT_EQ(restored->views.first().current_page, 1);
    EXPECT_EQ(restored->views.first().page_size, 0);
    EXPECT_EQ(restored->views.first().sort_order, Qt::AscendingOrder);
    EXPECT_EQ(restored->views.first().filters.search_field, SearchField::AllFields);
    EXPECT_TRUE(restored->views.first().filters.live_tailing_enabled);
}

/**
 * @brief Verifies that an empty JSON document is not treated as a valid session.
 */
TEST(SessionCodecTest, RejectsEmptyDocuments)
{
    EXPECT_FALSE(SessionCodec::from_json(QJsonObject{}).has_value());
}

/**
 * @brief Verifies deterministic serialization of unordered filter sets.
 */
TEST(SessionCodecTest, WritesSetValuesInStableOrder)
{
    SessionViewState view;
    view.filters.log_levels = {QStringLiteral("WARN"), QStringLiteral("DEBUG"),
                               QStringLiteral("INFO")};
    view.filters.hidden_files = {QStringLiteral("z.log"), QStringLiteral("a.log")};

    SessionState state;
    state.views.append(view);

    const QJsonObject json = SessionCodec::to_json(state);
    const QJsonObject filters = json.value(QStringLiteral("views"))
                                    .toArray()
                                    .first()
                                    .toObject()
                                    .value(QStringLiteral("filters"))
                                    .toObject();
    const QJsonArray levels = filters.value(QStringLiteral("log_levels")).toArray();
    const QJsonArray hidden_files = filters.value(QStringLiteral("hidden_files")).toArray();

    EXPECT_EQ(levels.at(0).toString(), QStringLiteral("DEBUG"));
    EXPECT_EQ(levels.at(1).toString(), QStringLiteral("INFO"));
    EXPECT_EQ(levels.at(2).toString(), QStringLiteral("WARN"));
    EXPECT_EQ(hidden_files.at(0).toString(), QStringLiteral("a.log"));
    EXPECT_EQ(hidden_files.at(1).toString(), QStringLiteral("z.log"));
}
}  // namespace
