#pragma once

#include <QString>
#include <QUuid>
#include <QVector>

#include "Qt-LogViewer/Views/Shared/TabWidget.h"

class LogViewWidget;

/**
 * @class LogTabWidget
 * @brief Specialized TabWidget for managing LogViewWidget tabs.
 *
 */
class LogTabWidget: public TabWidget
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs a LogTabWidget object.
         * @param parent The parent widget, or nullptr.
         */
        explicit LogTabWidget(QWidget* parent = nullptr);

        /**
         * @brief Destroys the LogTabWidget object.
         */
        ~LogTabWidget() override = default;

        /**
         * @brief Applies default tab behavior for log views.
         */
        auto setup_default_behavior() -> void;

        /**
         * @brief Returns the LogViewWidget at a given tab index.
         * @param index The tab index.
         * @return Pointer to LogViewWidget if present; otherwise nullptr.
         */
        [[nodiscard]] auto log_view_at(int index) const -> LogViewWidget*;

        /**
         * @brief Returns the current LogViewWidget.
         * @return Pointer to current LogViewWidget if present; otherwise nullptr.
         */
        [[nodiscard]] auto current_log_view() const -> LogViewWidget*;

        /**
         * @brief Finds the tab index for a given view id.
         * @param view_id The view identifier.
         * @return Tab index if found; otherwise -1.
         */
        [[nodiscard]] auto find_view_index(const QUuid& view_id) const -> int;

        /**
         * @brief Sets file paths on a view tab identified by view id.
         * @param view_id The view identifier.
         * @param file_paths File paths to set on the target LogViewWidget.
         * @return True if the target tab was found and updated; otherwise false.
         */
        auto set_view_file_paths(const QUuid& view_id, const QVector<QString>& file_paths) -> bool;

        /**
         * @brief Removes a view tab by its view id.
         * @param view_id The view identifier.
         * @return True if a tab was removed; otherwise false.
         */
        auto remove_view_tab_by_id(const QUuid& view_id) -> bool;

        /**
         * @brief Adds a LogViewWidget as tab, selects it, and optionally auto-resizes columns.
         * @param log_view_widget The view widget to add.
         * @param tab_title The tab title.
         * @param auto_resize_columns True to auto-resize after selecting.
         * @return New tab index, or -1 if input is invalid.
         */
        auto add_log_view_tab(LogViewWidget* log_view_widget, const QString& tab_title,
                              bool auto_resize_columns = true) -> int;

        /**
         * @brief Removes all tabs.
         */
        auto close_all_view_tabs() -> void;

        /**
         * @brief Resizes columns of the current LogViewWidget table.
         */
        auto auto_resize_current_columns() -> void;
};
