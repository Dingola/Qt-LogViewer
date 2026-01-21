#pragma once

#include <QAbstractItemModel>
#include <QItemSelectionModel>
#include <QWidget>

namespace Ui
{
class StartPageWidget;
}

/**
 * @file StartPageWidget.h
 * @brief Declares the `StartPageWidget` used on the start page to present primary actions
 *        and recent lists (files and sessions).
 *
 * The widget loads its layout from `Forms/StartPageWidget.ui`. Recent items are displayed
 * as list entries (QListView) with a custom delegate to render filename/name, path/id and
 * last-opened date with distinct styles.
 */
class StartPageWidget: public QWidget
{
        Q_OBJECT

    public:
        /**
         * @brief Construct the StartPageWidget UI wrapper.
         * @param parent Parent widget.
         */
        explicit StartPageWidget(QWidget* parent = nullptr);

        /**
         * @brief Destroy the StartPageWidget.
         */
        ~StartPageWidget() override;

        /**
         * @brief Assign the model for the recent files list view.
         * @param model QAbstractItemModel exposing roles: file_path, file_name, app_name,
         * last_opened.
         */
        auto set_recent_files_model(QAbstractItemModel* model) -> void;

        /**
         * @brief Assign the model for the recent sessions list view.
         * @param model QAbstractItemModel exposing roles: name, last_opened, id.
         */
        auto set_recent_sessions_model(QAbstractItemModel* model) -> void;

    signals:
        /**
         * @brief User requested to open a log file via file dialog.
         */
        void open_log_file_requested();

        /**
         * @brief User requested to open a specific recent file.
         * @param file_path Selected file path.
         */
        void open_recent_file_requested(const QString& file_path);

        /**
         * @brief User requested to clear the recent files list.
         */
        void clear_recent_files_requested();

        /**
         * @brief User requested to open a specific session.
         * @param session_id Selected session id.
         */
        void open_session_requested(const QString& session_id);

        /**
         * @brief User requested to open a specific recent session.
         * @param session_id Selected session id.
         */
        void open_recent_session_requested(const QString& session_id);

        /**
         * @brief User requested to reopen the last session.
         */
        void reopen_last_session_requested();

        /**
         * @brief User requested to delete a specific session.
         * @param session_id Selected session id.
         */
        void delete_session_requested(const QString& session_id);

    private:
        /**
         * @brief Configure sensible defaults for the recent lists.
         */
        auto setup_table_views() -> void;

        /**
         * @brief Wire button clicks to signals.
         */
        auto setup_connections() -> void;

        /**
         * @brief Helper to read selected file_path from `recentFilesView`.
         * @return Selected file path or empty.
         */
        [[nodiscard]] auto get_selected_recent_file_path() const -> QString;

        /**
         * @brief Helper to read selected session id from `recentSessionsView`.
         * @return Selected session id or empty.
         */
        [[nodiscard]] auto get_selected_recent_session_id() const -> QString;

        /**
         * @brief Update enabled state of action buttons based on models and selection.
         */
        auto update_buttons_state() -> void;

    private:
        Ui::StartPageWidget* ui{nullptr};
};
