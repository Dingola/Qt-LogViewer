#pragma once

#include <QByteArray>
#include <QList>
#include <QObject>
#include <QPointer>

class QAction;
class QDockWidget;
class QMainWindow;

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
         * While suspended, the native dock toggle actions are disabled. Repeating the current
         * state has no effect.
         */
        auto set_docks_suspended(bool suspended) -> void;

    private:
        QMainWindow* m_main_window = nullptr;
        QList<QPointer<QDockWidget>> m_docks;
        QByteArray m_suspended_layout_state;
        bool m_docks_suspended = false;
};
