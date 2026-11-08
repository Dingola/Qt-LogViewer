#pragma once

#include <QJsonObject>
#include <QObject>
#include <QSet>
#include <QVector>

#include "Qt-LogViewer/Models/LogFileInfo.h"
#include "Qt-LogViewer/Models/SessionTypes.h"
#include "QtCommonLib/Collections/MostRecentlyUsedList.h"

class SessionRepository;

/**
 * @file RecentFilesService.h
 * @brief Declares the domain service for persisted recent log files.
 */

/**
 * @class RecentFilesService
 * @brief Owns recent-file MRU behavior while preserving the shared session index document.
 */
class RecentFilesService final: public QObject
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs a recent-files service.
         * @param repository Repository containing the shared index document.
         * @param maximum_size Maximum retained entry count, or zero when unbounded.
         * @param parent Optional QObject parent.
         */
        explicit RecentFilesService(SessionRepository* repository, qsizetype maximum_size = 0,
                                    QObject* parent = nullptr);

        /**
         * @brief Loads existing recent-file records from the unchanged index schema.
         */
        auto initialize_from_storage() -> void;

        /**
         * @brief Returns recent files in most-recently-used order.
         * @return Copy of the current recent-file records.
         */
        [[nodiscard]] auto get_recent_log_files() const -> QVector<RecentLogFileRecord>;

        /**
         * @brief Adds or updates a recent log file and persists the resulting order.
         * @param file_info File metadata used to create the recent-file record.
         */
        auto add_recent_log_file(const LogFileInfo& file_info) -> void;

        /**
         * @brief Clears all recent-file metadata and persists the empty list.
         */
        auto clear_recent_log_files() -> void;

        /**
         * @brief Removes matching recent-file metadata without deleting source files.
         * @param file_paths Paths identifying entries to remove.
         * @return Number of removed entries.
         */
        auto remove_recent_log_files(const QSet<QString>& file_paths) -> int;

        /**
         * @brief Replaces one recent-file path while preserving its remaining metadata.
         * @param previous_file_path Path identifying the existing entry.
         * @param replacement_file_path Readable replacement path selected by the user.
         * @return True when the entry changed and was persisted; otherwise false.
         */
        auto relink_recent_log_file(const QString& previous_file_path,
                                    const QString& replacement_file_path) -> bool;

    signals:
        /**
         * @brief Emitted after the recent-file collection changes.
         * @param items Current recent-file records in MRU order.
         */
        void recent_log_files_changed(const QVector<RecentLogFileRecord>& items);

    private:
        /**
         * @brief Produces a platform-appropriate stable comparison key for a path.
         * @param file_path Persisted or incoming local path.
         * @return Clean absolute path key.
         */
        [[nodiscard]] static auto path_key(const QString& file_path) -> QString;

        /**
         * @brief Converts a recent-file record to its persisted JSON representation.
         * @param record Record to convert.
         * @return JSON object using the existing index schema.
         */
        [[nodiscard]] static auto to_json(const RecentLogFileRecord& record) -> QJsonObject;

        /**
         * @brief Parses a recent-file record from the existing index schema.
         * @param object Persisted recent-file object.
         * @return Parsed recent-file record.
         */
        [[nodiscard]] static auto from_json(const QJsonObject& object) -> RecentLogFileRecord;

        /**
         * @brief Persists recent files while retaining every unrelated root field.
         */
        auto persist() -> void;

    private:
        SessionRepository* m_repository{nullptr};
        QtCommonLib::MostRecentlyUsedList<RecentLogFileRecord, QString> m_recent_files;
};
