#pragma once

#include <QAbstractItemModel>
#include <QMap>
#include <QMenu>
#include <QModelIndex>
#include <QSet>
#include <QString>
#include <QToolButton>
#include <QUuid>
#include <QVector>
#include <QWidget>

#include "Qt-LogViewer/Models/SearchFields.h"

namespace Ui
{
class LogViewWidget;
}

class LogFilterWidget;
class LogTableView;

/**
 * @class LogViewWidget
 * @brief Combines a database-backed log table with per-view controls.
 *
 * Responsibilities:
 * - Display the current database page in a LogTableView.
 * - Forward application-name and log-level filter changes.
 * - Apply presentation-only search highlighting to displayed cells.
 * - Manage per-file show-only, visibility and removal actions.
 * - Manage the per-view live-tailing control.
 */
class LogViewWidget: public QWidget
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs a LogViewWidget object.
         *
         * Initializes the UI, forwards signals, and prepares the composite for use.
         *
         * @param parent The parent widget, or nullptr if top-level.
         */
        explicit LogViewWidget(QWidget* parent = nullptr);

        /**
         * @brief Destroys the LogViewWidget object.
         *
         * Cleans up UI resources.
         */
        ~LogViewWidget() override;

        /**
         * @brief Sets the unique view id associated with this widget.
         *
         * Emits `view_id_changed` if the id differs from the previous one.
         *
         * @param view_id The new view id.
         */
        auto set_view_id(const QUuid& view_id) -> void;

        /**
         * @brief Gets the unique view id associated with this widget.
         *
         * @return The current view id (may be null QUuid if not set).
         */
        [[nodiscard]] auto get_view_id() const -> QUuid;

        /**
         * @brief Sets the model displayed by the internal log table.
         * @param model Model containing the current database page.
         */
        auto set_model(QAbstractItemModel* model) -> void;

        /**
         * @brief Convenience method: resizes columns in the internal table view.
         */
        auto auto_resize_columns() -> void;

        /**
         * @brief Sets the application names for the filter widget.
         * @param app_names Set of application names.
         */
        auto set_app_names(const QSet<QString>& app_names) -> void;

        /**
         * @brief Selects a specific application name in the filter widget if present.
         * @param app_name Application name to select.
         */
        auto set_current_app_name_filter(const QString& app_name) -> void;

        /**
         * @brief Sets available log level names for creating level items.
         * @param log_levels List of log level names.
         */
        auto set_available_log_levels(const QVector<QString>& log_levels) -> void;

        /**
         * @brief Sets which log levels are currently checked.
         * @param levels Set of log level names to check.
         */
        auto set_log_levels(const QSet<QString>& levels) -> void;

        /**
         * @brief Updates per-log-level counts displayed in the filter items.
         * @param level_counts Map of level name to count.
         */
        auto set_log_level_counts(const QMap<QString, int>& level_counts) -> void;

        /**
         * @brief Shows or hides the filter widget area.
         *
         * @param visible True to show, false to hide.
         */
        auto set_filter_widget_visible(bool visible) -> void;

        /**
         * @brief Updates the visibility state of files in the view.
         * @param show_only_file File displayed exclusively, or an empty string.
         * @param hidden_files Files explicitly hidden from the view.
         */
        auto set_file_visibility_state(const QString& show_only_file,
                                       const QSet<QString>& hidden_files) -> void;

        /**
         * @brief Resets all filter related UI elements (apps + levels).
         *
         * Clears application names, log levels, counts.
         */
        auto clear_filters() -> void;

        /**
         * @brief Returns the currently selected application name.
         * @return Current application name (empty if "Show All Apps" or none selected).
         */
        [[nodiscard]] auto get_current_app_name() const -> QString;

        /**
         * @brief Returns the set of currently selected log levels.
         * @return Set of selected log level identifiers.
         */
        [[nodiscard]] auto get_current_log_levels() const -> QSet<QString>;

        /**
         * @brief Returns a pointer to the internal LogFilterWidget.
         * @return Pointer to LogFilterWidget.
         */
        [[nodiscard]] auto get_filter_widget() const -> LogFilterWidget*;

        /**
         * @brief Returns a pointer to the internal LogTableView.
         * @return Pointer to LogTableView.
         */
        [[nodiscard]] auto get_table_view() const -> LogTableView*;

        /**
         * @brief Updates the list of file paths loaded in the current view.
         *
         * This list is used to populate the "Files in View" dropdown menu with per-file actions.
         * Call this when the current view adds/removes files.
         *
         * @param file_paths Vector of absolute file paths currently loaded in this view.
         */
        auto set_view_file_paths(const QVector<QString>& file_paths) -> void;

        /**
         * @brief Sets the live tail checkbox state without emitting a user-change signal.
         * @param enabled True to check the control.
         */
        auto set_live_tailing_enabled(bool enabled) -> void;

        /**
         * @brief Returns the live tail checkbox state.
         * @return True when live tailing is enabled.
         */
        [[nodiscard]] auto get_live_tailing_enabled() const -> bool;

        /**
         * @brief Sets the search used for highlighting displayed log values.
         * @param text Search text or regular expression.
         * @param field Field whose cells are highlighted.
         * @param use_regex Whether text is interpreted as a regular expression.
         */
        auto set_search_highlight(const QString& text, SearchField field, bool use_regex) -> void;

    signals:
        /**
         * @brief Emitted when the application filter selection changes.
         * @param app_name Selected application name (may be empty for "all").
         */
        void app_filter_changed(const QString& app_name);

        /**
         * @brief Emitted when the log level filter selection changes.
         * @param levels Set of selected log levels.
         */
        void log_level_filter_changed(const QSet<QString>& levels);

        /**
         * @brief Emitted when the view id is changed.
         * @param view_id New view id value.
         */
        void view_id_changed(const QUuid& view_id);

        /**
         * @brief Emitted when the current row in the table view changes.
         * @param current Current index.
         * @param previous Previous index.
         */
        void current_row_changed(const QModelIndex& current, const QModelIndex& previous);

        /**
         * @brief Emitted when the user requests to show only a specific file in the current view.
         * @param file_path Target file path.
         */
        void show_only_file_requested(const QString& file_path);

        /**
         * @brief Emitted when the user requests to toggle a file's visibility (hide/show).
         * @param file_path Target file path.
         */
        void toggle_visibility_requested(const QString& file_path);

        /**
         * @brief Emitted when the user requests to remove a specific file from the current view.
         * @param file_path Target file path.
         */
        void remove_file_requested(const QString& file_path);

        /**
         * @brief Emitted when the user changes the live tail state.
         * @param enabled True when tailing should be enabled.
         */
        void live_tailing_toggled(bool enabled);

    protected:
        /**
         * @brief Handles language change and other UI change events.
         * @param event The change event.
         */
        auto changeEvent(QEvent* event) -> void override;

    private:
        /**
         * @brief Creates the "Files in View" tool button and attaches the dropdown menu.
         *
         * The button is inserted into the top-level layout and configured for instant popup.
         */
        auto setup_files_menu() -> void;

        /**
         * @brief Rebuilds the "Files in View" dropdown menu from `m_view_file_paths`.
         *
         * Adds a "Show All Files" action and one per-file row with inline actions
         * (Show Only, Hide, Remove).
         */
        auto rebuild_files_menu() -> void;

        /**
         * @brief Refreshes file-menu actions from the stored visibility state.
         */
        auto refresh_files_menu_states() -> void;

    private:
        Ui::LogViewWidget* ui;
        QUuid m_view_id;
        QMenu* m_files_menu = nullptr;
        QVector<QString> m_view_file_paths;
        QString m_show_only_file;
        QSet<QString> m_hidden_files;
};
