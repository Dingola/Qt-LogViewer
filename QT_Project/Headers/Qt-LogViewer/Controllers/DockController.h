#pragma once

#include <QByteArray>
#include <QHash>
#include <QList>
#include <QObject>
#include <QPointer>
#include <QSize>

class QAction;
class QDockWidget;
class QMainWindow;
class QEvent;

/**
 * @file DockController.h
 * @brief Controller responsible for registered dock widgets and temporary dock suspension.
 */

/**
 * @class DockController
 * @brief Coordinates registered dock widgets without knowing application-specific dock identities.
 *
 * The controller exposes each registered dock widget's native toggle action and can temporarily
 * suspend all docks. Suspending captures the current QMainWindow layout before hiding the docks.
 * Resuming restores that captured layout.
 */
class DockController: public QObject
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs a DockController.
         * @param main_window Window whose dock layout is managed.
         * @param parent Optional QObject parent.
         */
        explicit DockController(QMainWindow* main_window, QObject* parent = nullptr);

        /**
         * @brief Registers a dock widget.
         * @param dock_widget Dock widget to register.
         *
         * Null dock widgets and dock widgets that are already registered are ignored. A dock
         * registered while docks are suspended is hidden and its toggle action is disabled.
         */
        auto register_dock(QDockWidget* dock_widget) -> void;

        /**
         * @brief Returns the native toggle actions of all registered dock widgets.
         * @return Toggle actions in registration order.
         */
        [[nodiscard]] auto get_toggle_actions() const -> QList<QAction*>;

        /**
         * @brief Temporarily hides or restores all registered docks.
         * @param suspended True to capture the current layout and hide all docks; false to restore
         * the captured layout.
         *
         * While suspended, the native dock toggle actions are disabled. Repeating suspension
         * reapplies the hidden state without replacing the captured layout.
         */
        auto set_docks_suspended(bool suspended) -> void;

        /**
         * @brief Restores preferred dock extents after QMainWindow handled a resize event.
         * @param old_size Previous QMainWindow size.
         * @param new_size New QMainWindow size.
         */
        auto handle_main_window_resize(const QSize& old_size, const QSize& new_size) -> void;

        /**
         * @brief Captures the currently visible dock layout as the preferred layout.
         */
        auto capture_current_sizes() -> void;

    protected:
        /**
         * @brief Tracks user-selected dock sizes.
         * @param watched Object receiving the event.
         * @param event Event being delivered.
         * @return The base QObject event-filter result.
         */
        bool eventFilter(QObject* watched, QEvent* event) override;

    private:
        /**
         * @brief Applies remembered dock extents during the current layout pass.
         * @param restore_width True to restore the preferred width of each dock.
         * @param restore_height True to restore the preferred height of each dock.
         * @return True if any dock extents were applied; false if no docks were visible or all
         */
        auto restore_preferred_sizes(bool restore_width, bool restore_height) -> bool;

    private:
        QMainWindow* m_main_window = nullptr;
        QList<QPointer<QDockWidget>> m_docks;
        QHash<QDockWidget*, QSize> m_preferred_sizes;
        QSize m_observed_main_window_size;
        QByteArray m_suspended_layout_state;
        bool m_docks_suspended = false;
        bool m_applying_sizes = false;
        bool m_main_window_resize_active = false;
        quint64 m_resize_generation = 0;
};
