#pragma once

#include <QString>
#include <QVector>
#include <QWidget>
#include <optional>

#include "Qt-LogViewer/Services/LogParseOutcome.h"
#include "Qt-LogViewer/Services/LogParsingProfile.h"

namespace Ui
{
class LogImportWidget;
}

class LogViewerController;
class LogViewerSettings;
class QModelIndex;
class QStandardItemModel;
class QTimer;

/**
 * @file LogImportWidget.h
 * @brief Declares the LogImportWidget class for selecting or creating a parsing profile before
 * importing a log file.
 */

/**
 * @class LogImportWidget
 * @brief Selects an existing parsing profile or creates a new profile before importing a file.
 *
 * The widget performs only bounded preview parsing. The actual asynchronous import is started by
 * the owner after the widget emits import_requested().
 */
class LogImportWidget final: public QWidget
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs a log import widget for one file.
         * @param file_path File whose records are previewed and later imported.
         * @param settings Application settings containing reusable parsing profiles.
         * @param controller Controller used for bounded, side-effect-free preview parsing.
         * @param parent Parent widget responsible for ownership.
         */
        explicit LogImportWidget(QString file_path, LogViewerSettings& settings,
                                 const LogViewerController& controller, QWidget* parent = nullptr);

        /** @brief Destroys the widget and its generated user interface. */
        ~LogImportWidget() override;

        /**
         * @brief Returns the file selected for this import.
         * @return File path supplied during construction.
         */
        [[nodiscard]] auto get_file_path() const noexcept -> const QString&;

        /**
         * @brief Returns the profile confirmed by the user.
         * @return Confirmed profile after an import request, or std::nullopt beforehand.
         */
        [[nodiscard]] auto get_selected_profile() const -> std::optional<LogParsingProfile>;

    signals:
        /** @brief Emitted after a valid parsing profile has been confirmed. */
        void import_requested();

        /** @brief Emitted when the pending import should be discarded. */
        void cancel_requested();

    private:
        /** @brief Loads persisted profiles and initializes existing- or new-profile mode. */
        auto load_profiles() -> void;

        /**
         * @brief Displays the profile selected by combo-box index.
         * @param index Index in the persisted profile collection.
         */
        auto select_profile(int index) -> void;

        /** @brief Clears the editor and enables creation of a new profile. */
        auto begin_new_profile() -> void;

        /** @brief Parses the configured number of records and refreshes the preview table. */
        auto refresh_preview() -> void;

        /** @brief Shows raw source records while no usable profile format is available. */
        auto show_raw_preview() -> void;

        /** @brief Restarts the delayed preview refresh after an editor change. */
        auto schedule_preview_refresh() -> void;

        /**
         * @brief Inserts a parser placeholder at the current format-editor cursor position.
         * @param field_id Stable or custom field identifier without braces.
         */
        auto insert_field_placeholder(const QString& field_id) -> void;

        /** @brief Requests a custom field name and inserts its placeholder when valid. */
        auto insert_custom_field_placeholder() -> void;

        /** @brief Validates and stores the selected profile before requesting the import. */
        auto request_import() -> void;

        /** @brief Updates editor state, validation text and import-button availability. */
        auto update_widget_state() -> void;

        /**
         * @brief Displays or hides the current profile-validation message.
         * @param message Message to display, or an empty string to hide the hint.
         */
        auto set_validation_message(const QString& message) -> void;

        /**
         * @brief Displays or hides the current preview summary.
         * @param message Summary to display, or an empty string to hide the hint.
         */
        auto set_preview_summary(const QString& message) -> void;

        /**
         * @brief Updates import availability and explains a disabled state through a tooltip.
         * @param profile Current valid preview profile, or std::nullopt when none is available.
         * @param unavailable_reason Current parser or preview validation message.
         */
        auto update_import_button(const std::optional<LogParsingProfile>& profile,
                                  const QString& unavailable_reason) -> void;

        /**
         * @brief Creates the profile represented by the current controls.
         * @param error_message Optional destination for parser validation errors.
         * @return Current profile, or std::nullopt when the controls do not describe a valid one.
         */
        [[nodiscard]] auto create_current_profile(QString* error_message = nullptr) const
            -> std::optional<LogParsingProfile>;

        /**
         * @brief Replaces the raw-record list and retains its corresponding parse outcomes.
         * @param outcomes Successful and failed parse outcomes in source order.
         */
        auto populate_preview(const QVector<LogParseOutcome>& outcomes) -> void;

        /**
         * @brief Displays parsed values or failure details for one selected raw record.
         * @param current Current row in the raw-record table.
         */
        auto show_selected_outcome(const QModelIndex& current) -> void;

    private:
        Ui::LogImportWidget* ui;
        LogViewerSettings& m_settings;
        const LogViewerController& m_controller;
        QStandardItemModel* m_raw_records_model = nullptr;
        QStandardItemModel* m_parsed_fields_model = nullptr;
        QTimer* m_preview_timer = nullptr;
        QString m_file_path;
        QVector<LogParsingProfile> m_profiles;
        QVector<LogParseOutcome> m_preview_outcomes;
        std::optional<LogParsingProfile> m_selected_profile;
        bool m_creating_profile = false;
        bool m_preview_contains_successful_record = false;
        bool m_preview_matches_complete_record = false;
        bool m_showing_raw_preview = false;
        bool m_showing_partial_preview = false;
};
