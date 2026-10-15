#pragma once

#include <QObject>

class LogDetailsWidget;
class LogFilterBarWidget;
class LogPageCoordinator;
class LogTabWidget;

/**
 * @file WorkspaceShortcutController.h
 * @brief Declares window-scoped keyboard coordination for the log workspace.
 */

/**
 * @class WorkspaceShortcutController
 * @brief Binds global workspace shortcuts to focused widget and controller actions.
 *
 * Widgets retain their local operations, while this controller owns shortcuts that cross widget
 * boundaries or must remain available throughout the application window.
 */
class WorkspaceShortcutController final: public QObject
{
    public:
        /**
         * @brief Installs workspace shortcuts on the tab widget's window.
         * @param pages Pagination operations used for result navigation.
         * @param tab_widget Workspace tab container and shortcut host.
         * @param filter_bar Shared search controls.
         * @param details_widget Shared selected-entry inspector.
         * @param parent Optional QObject parent.
         */
        explicit WorkspaceShortcutController(LogPageCoordinator* pages, LogTabWidget* tab_widget,
                                             LogFilterBarWidget* filter_bar,
                                             LogDetailsWidget* details_widget,
                                             QObject* parent = nullptr);

    private:
        /** @brief Installs the window-scoped keyboard shortcuts. */
        auto setup_shortcuts() -> void;

        /** @brief Focuses the active log table when one is available. */
        auto focus_active_table() -> void;

        /**
         * @brief Selects the next or previous filtered entry, including adjacent pages.
         * @param direction Positive for the next entry and negative for the previous entry.
         */
        auto select_relative_result(int direction) -> void;

        /**
         * @brief Activates the next or previous workspace tab with wraparound.
         * @param direction Positive for the next tab and negative for the previous tab.
         */
        auto switch_relative_tab(int direction) -> void;

        LogPageCoordinator* m_pages{nullptr};
        LogTabWidget* m_tab_widget{nullptr};
        LogFilterBarWidget* m_filter_bar{nullptr};
        LogDetailsWidget* m_details_widget{nullptr};
};
