#pragma once

#include <QJsonObject>
#include <QString>
#include <optional>

#include "Qt-LogViewer/Models/SessionTypes.h"

/**
 * @file SessionCodec.h
 * @brief Declares conversion between typed session state and persisted JSON documents.
 */

/**
 * @class SessionCodec
 * @brief Converts typed session state to and from the persisted JSON schema.
 *
 * The codec owns JSON field names, compatibility defaults, schema versioning and conversion
 * between the one-based runtime page number and the zero-based persisted page index.
 */
class SessionCodec final
{
    public:
        /**
         * @brief Returns the current session JSON schema version.
         */
        [[nodiscard]] static auto get_schema_version() -> int;

        /**
         * @brief Serializes a typed session state.
         * @param state Runtime session state.
         * @return JSON document using the current schema.
         */
        [[nodiscard]] static auto to_json(const SessionState& state) -> QJsonObject;

        /**
         * @brief Deserializes a session JSON document.
         * @param object Persisted JSON document.
         * @param fallback_session_id ID used for legacy documents without an embedded ID.
         * @return Typed state, or no value for an empty document.
         */
        [[nodiscard]] static auto from_json(const QJsonObject& object,
                                            const QString& fallback_session_id = QString())
            -> std::optional<SessionState>;

    private:
        /**
         * @brief Serializes one log file and its optional parsing profile.
         * @param file_info Log file metadata to serialize.
         * @param parsing_profile_id Optional parsing-profile identifier.
         * @return JSON object containing the persisted file fields.
         */
        [[nodiscard]] static auto file_to_json(
            const LogFileInfo& file_info, const QUuid& parsing_profile_id = QUuid()) -> QJsonObject;

        /**
         * @brief Deserializes log file metadata from JSON.
         * @param object JSON object containing persisted file fields.
         * @return Parsed log file metadata.
         */
        [[nodiscard]] static auto file_from_json(const QJsonObject& object) -> LogFileInfo;

        /**
         * @brief Serializes one view state including files, filters, paging, and sorting.
         * @param state Runtime view state to serialize.
         * @return JSON object containing the persisted view fields.
         */
        [[nodiscard]] static auto view_to_json(const SessionViewState& state) -> QJsonObject;

        /**
         * @brief Deserializes one view state and applies compatibility defaults.
         * @param object JSON object containing persisted view fields.
         * @return Parsed runtime view state.
         */
        [[nodiscard]] static auto view_from_json(const QJsonObject& object) -> SessionViewState;
};
