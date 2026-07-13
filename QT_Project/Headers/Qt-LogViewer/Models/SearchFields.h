#pragma once

#include <QLatin1StringView>
#include <QMetaType>
#include <QString>

/**
 * @def SEARCH_FIELD_LIST
 * @brief X-Macro definition listing all available search fields.
 *
 * Each entry defines:
 * - Enum identifier (e.g., AllFields)
 * - Stable string key (e.g., "all_fields")
 */
#define SEARCH_FIELD_LIST      \
    X(AllFields, "all_fields") \
    X(Message, "message")      \
    X(Level, "level")          \
    X(AppName, "app_name")

/**
 * @enum SearchField
 * @brief Enumeration of available search fields in the log viewer.
 */
enum class SearchField
{
#define X(name, key) name,
    SEARCH_FIELD_LIST
#undef X
        Count  ///< Total number of search fields.
};

Q_DECLARE_METATYPE(SearchField)

/**
 * @brief Array of stable string keys for each SearchField.
 *
 * Indices correspond to the underlying integer values of the enum.
 */
constexpr inline const char* SearchFieldKeys[] = {
#define X(name, key) key,
    SEARCH_FIELD_LIST
#undef X
};

/**
 * @brief Number of search fields.
 *
 * Computed at compile-time from the SearchField::Count sentinel.
 */
constexpr inline int SearchFieldCount = static_cast<int>(SearchField::Count);

/**
 * @brief Converts a SearchField to its stable key.
 *
 * @param field The search field to convert.
 * @return The stable key as QLatin1StringView. Returns an empty view if the field is invalid.
 */
[[nodiscard]] constexpr inline auto to_latin1_string_view(SearchField field) noexcept
    -> QLatin1StringView
{
    QLatin1StringView value;
    const int index = static_cast<int>(field);

    if (index >= 0 && index < SearchFieldCount)
    {
        value = QLatin1StringView{SearchFieldKeys[index]};
    }

    return value;
}

/**
 * @brief Converts a SearchField to its stable key.
 *
 * @param field The search field to convert.
 * @return The stable key as QString. Returns an empty QString if the field is invalid.
 */
[[nodiscard]] inline auto to_string(SearchField field) -> QString
{
    QString value = to_latin1_string_view(field);
    return value;
}

/**
 * @brief Converts a stable key to SearchField.
 *
 * Performs case-insensitive comparison against all registered keys.
 *
 * @param value The stable key to convert.
 * @return Matching SearchField, or SearchField::AllFields as fallback if no match is found.
 */
[[nodiscard]] inline auto from_latin1_string_view(QLatin1StringView value) noexcept -> SearchField
{
    SearchField field = SearchField::AllFields;
    bool found = false;

    for (int i = 0; i < SearchFieldCount && !found; ++i)
    {
        if (value.compare(QLatin1StringView{SearchFieldKeys[i]}, Qt::CaseInsensitive) == 0)
        {
            field = static_cast<SearchField>(i);
            found = true;
        }
    }

    return field;
}

#undef SEARCH_FIELD_LIST
