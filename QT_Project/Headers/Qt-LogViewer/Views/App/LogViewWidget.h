#pragma once

#include <QAbstractItemModel>
#include <QMap>
#include <QSet>
#include <QString>
#include <QUuid>
#include <QVector>
#include <QWidget>

namespace Ui
{
class LogViewWidget;
}

class LogFilterWidget;
class LogTableView;

/**
 * @class LogViewWidget
 * @brief Composite widget encapsulating a log view (table) and its per-view filters.
 *
 * This widget combines a `LogFilterWidget` (application + log level filters) with a
 * `LogTableView` that displays the filtered log entries.
 *
 * Responsibilities:
 * - Provide an API to configure and query filter state.
 * - Forward filter change signals.
 * - Manage a per-view identifier (QUuid) for controller coordination.
 * - Expose the internal table view for selection/model access.
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
         * @brief Sets the model on the internal log table view.
         *
         * Call this after creating the corresponding proxies in the controller layer.
         *
         * @param model The model to assign (e.g. paging or sort/filter proxy).
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

    protected:
        /**
         * @brief Handles language change and other UI change events.
         * @param event The change event.
         */
        auto changeEvent(QEvent* event) -> void override;

    private:
        Ui::LogViewWidget* ui;
        QUuid m_view_id;
};
