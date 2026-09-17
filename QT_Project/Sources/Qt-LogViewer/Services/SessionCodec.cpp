#include "Qt-LogViewer/Services/SessionCodec.h"

#include <QFileInfo>
#include <QJsonArray>
#include <QStringList>
#include <algorithm>

/**
 * @file SessionCodec.cpp
 * @brief Implements the session JSON format and its compatibility conversions.
 */

namespace
{
constexpr int k_schema_version = 1;

/**
 * @brief Returns deterministic ordering for values stored in an unordered set.
 * @param values Values to sort.
 * @return Case-sensitive sorted values.
 */
[[nodiscard]] auto sorted_values(const QSet<QString>& values) -> QStringList
{
    QStringList result(values.begin(), values.end());
    result.sort(Qt::CaseSensitive);
    return result;
}
}  // namespace

/**
 * @brief Returns the current session JSON schema version.
 * @return Schema version written by to_json().
 */
auto SessionCodec::get_schema_version() -> int
{
    return k_schema_version;
}

/**
 * @brief Serializes a complete typed session state to JSON.
 * @param state Runtime session state.
 * @return JSON document using the current session schema.
 */
auto SessionCodec::to_json(const SessionState& state) -> QJsonObject
{
    QJsonArray views;
    for (const SessionViewState& view: state.views)
    {
        views.append(view_to_json(view));
    }

    QJsonArray explorer_files;
    for (const LogFileInfo& file_info: state.explorer_files)
    {
        explorer_files.append(file_to_json(file_info));
    }

    QJsonObject object;
    object.insert(QStringLiteral("schema_version"), get_schema_version());
    object.insert(QStringLiteral("id"), state.id);
    object.insert(QStringLiteral("name"), state.name);
    object.insert(QStringLiteral("views"), views);
    object.insert(QStringLiteral("explorer_files"), explorer_files);
    return object;
}

/**
 * @brief Deserializes a session JSON document and applies compatibility defaults.
 * @param object Persisted session document.
 * @param fallback_session_id ID used when the document does not contain one.
 * @return Typed session state, or no value when the document is empty.
 */
auto SessionCodec::from_json(const QJsonObject& object,
                             const QString& fallback_session_id) -> std::optional<SessionState>
{
    if (object.isEmpty())
    {
        return std::nullopt;
    }

    SessionState state;
    state.schema_version = object.value(QStringLiteral("schema_version")).toInt(1);
    state.id = object.value(QStringLiteral("id")).toString(fallback_session_id);
    state.name = object.value(QStringLiteral("name")).toString();

    const QJsonArray views = object.value(QStringLiteral("views")).toArray();
    state.views.reserve(views.size());
    for (const QJsonValue& value: views)
    {
        if (value.isObject())
        {
            state.views.append(view_from_json(value.toObject()));
        }
    }

    const QJsonArray explorer_files = object.value(QStringLiteral("explorer_files")).toArray();
    for (const QJsonValue& value: explorer_files)
    {
        if (value.isObject())
        {
            const LogFileInfo file_info = file_from_json(value.toObject());
            if (!file_info.get_file_path().isEmpty())
            {
                state.explorer_files.append(file_info);
            }
        }
    }

    return state;
}

/**
 * @brief Serializes log file metadata and an optional parsing-profile association.
 * @param file_info Log file metadata to serialize.
 * @param parsing_profile_id Optional parsing-profile identifier.
 * @return JSON object containing the persisted file fields.
 */
auto SessionCodec::file_to_json(const LogFileInfo& file_info,
                                const QUuid& parsing_profile_id) -> QJsonObject
{
    QJsonObject object;
    object.insert(QStringLiteral("file_path"), file_info.get_file_path());
    object.insert(QStringLiteral("app_name"), file_info.get_app_name());

    if (!parsing_profile_id.isNull())
    {
        object.insert(QStringLiteral("parsing_profile_id"),
                      parsing_profile_id.toString(QUuid::WithoutBraces));
    }

    return object;
}

/**
 * @brief Deserializes log file metadata from JSON.
 * @param object JSON object containing persisted file fields.
 * @return Parsed log file metadata.
 */
auto SessionCodec::file_from_json(const QJsonObject& object) -> LogFileInfo
{
    return LogFileInfo(object.value(QStringLiteral("file_path")).toString(),
                       object.value(QStringLiteral("app_name")).toString());
}

/**
 * @brief Serializes one view and converts its page number to the persisted zero-based index.
 * @param state Runtime view state.
 * @return JSON object containing files, filters, paging, sorting, and title.
 */
auto SessionCodec::view_to_json(const SessionViewState& state) -> QJsonObject
{
    QJsonArray files;
    for (const LogFileInfo& file_info: state.loaded_files)
    {
        const QString absolute_path = QFileInfo(file_info.get_file_path()).absoluteFilePath();
        const QUuid profile_id = state.file_parsing_profile_ids.value(absolute_path);
        files.append(file_to_json(file_info, profile_id));
    }

    QJsonArray log_levels;
    for (const QString& level: sorted_values(state.filters.log_levels))
    {
        log_levels.append(level);
    }

    QJsonArray hidden_files;
    for (const QString& file_path: sorted_values(state.filters.hidden_files))
    {
        hidden_files.append(file_path);
    }

    QJsonObject filters;
    filters.insert(QStringLiteral("app_name"), state.filters.app_name);
    filters.insert(QStringLiteral("log_levels"), log_levels);
    filters.insert(QStringLiteral("search_text"), state.filters.search_text);
    filters.insert(QStringLiteral("search_field"), to_string(state.filters.search_field));
    filters.insert(QStringLiteral("use_regex"), state.filters.use_regex);
    filters.insert(QStringLiteral("show_only_file"), state.filters.show_only_file);
    filters.insert(QStringLiteral("hidden_files"), hidden_files);
    filters.insert(QStringLiteral("live_tailing_enabled"), state.filters.live_tailing_enabled);

    QJsonObject object;
    object.insert(QStringLiteral("id"), state.id.toString(QUuid::WithoutBraces));
    object.insert(QStringLiteral("loaded_files"), files);
    object.insert(QStringLiteral("filters"), filters);
    object.insert(QStringLiteral("page_size"), state.page_size);
    object.insert(QStringLiteral("current_page"), std::max(0, state.current_page - 1));
    object.insert(QStringLiteral("sort_column"), state.sort_column);
    object.insert(QStringLiteral("sort_order"), state.sort_order == Qt::DescendingOrder
                                                    ? QStringLiteral("desc")
                                                    : QStringLiteral("asc"));
    object.insert(QStringLiteral("tab_title"), state.tab_title);
    return object;
}

/**
 * @brief Deserializes one view and converts its page index to a one-based runtime number.
 * @param object JSON object containing persisted view fields.
 * @return Parsed runtime view state with compatibility defaults applied.
 */
auto SessionCodec::view_from_json(const QJsonObject& object) -> SessionViewState
{
    SessionViewState state;
    state.id = QUuid(object.value(QStringLiteral("id")).toString());
    state.tab_title = object.value(QStringLiteral("tab_title")).toString();

    const QJsonArray files = object.value(QStringLiteral("loaded_files")).toArray();
    for (const QJsonValue& value: files)
    {
        if (!value.isObject())
        {
            continue;
        }

        const QJsonObject file_object = value.toObject();
        const LogFileInfo file_info = file_from_json(file_object);
        if (file_info.get_file_path().isEmpty())
        {
            continue;
        }

        state.loaded_files.append(file_info);

        const QUuid profile_id(file_object.value(QStringLiteral("parsing_profile_id")).toString());
        if (!profile_id.isNull())
        {
            state.file_parsing_profile_ids.insert(
                QFileInfo(file_info.get_file_path()).absoluteFilePath(), profile_id);
        }
    }

    const QJsonObject filters = object.value(QStringLiteral("filters")).toObject();
    state.filters.app_name = filters.value(QStringLiteral("app_name")).toString();
    state.filters.search_text = filters.value(QStringLiteral("search_text")).toString();

    const QByteArray search_field =
        filters.value(QStringLiteral("search_field")).toString().toLatin1();
    state.filters.search_field =
        from_latin1_string_view(QLatin1StringView(search_field.constData(), search_field.size()));
    state.filters.use_regex = filters.value(QStringLiteral("use_regex")).toBool();
    state.filters.show_only_file = filters.value(QStringLiteral("show_only_file")).toString();
    state.filters.live_tailing_enabled =
        filters.value(QStringLiteral("live_tailing_enabled")).toBool(true);

    const QJsonArray levels = filters.value(QStringLiteral("log_levels")).toArray();
    for (const QJsonValue& value: levels)
    {
        state.filters.log_levels.insert(value.toString());
    }

    const QJsonArray hidden_files = filters.value(QStringLiteral("hidden_files")).toArray();
    for (const QJsonValue& value: hidden_files)
    {
        state.filters.hidden_files.insert(value.toString());
    }

    state.page_size = object.value(QStringLiteral("page_size")).toInt();
    state.current_page = std::max(1, object.value(QStringLiteral("current_page")).toInt() + 1);
    state.sort_column = object.value(QStringLiteral("sort_column")).toInt();
    const QString sort_order =
        object.value(QStringLiteral("sort_order")).toString(QStringLiteral("asc"));
    state.sort_order = sort_order.compare(QStringLiteral("desc"), Qt::CaseInsensitive) == 0
                           ? Qt::DescendingOrder
                           : Qt::AscendingOrder;
    return state;
}
