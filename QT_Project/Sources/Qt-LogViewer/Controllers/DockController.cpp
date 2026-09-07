/**
 * @file DockController.cpp
 * @brief Implements DockController for registered dock widgets and layout suspension.
 */

#include "Qt-LogViewer/Controllers/DockController.h"

#include <QAction>
#include <QDockWidget>
#include <QMainWindow>
#include <utility>

/**
 * @brief Constructs a DockController.
 * @param main_window Window whose dock layout is managed.
 * @param parent Optional QObject parent.
 */
DockController::DockController(QMainWindow* main_window, QObject* parent)
    : QObject(parent), m_main_window(main_window)
{}

/**
 * @brief Registers a dock widget.
 * @param dock_widget Dock widget to register.
 */
auto DockController::register_dock(QDockWidget* dock_widget) -> void
{
    if (dock_widget != nullptr && !m_docks.contains(dock_widget))
    {
        m_docks.append(dock_widget);

        if (m_docks_suspended)
        {
            dock_widget->toggleViewAction()->setEnabled(false);
            dock_widget->setVisible(false);
        }
    }
}

/**
 * @brief Returns the native toggle actions of all registered dock widgets.
 * @return Toggle actions in registration order.
 */
auto DockController::get_toggle_actions() const -> QList<QAction*>
{
    QList<QAction*> toggle_actions;
    toggle_actions.reserve(m_docks.size());

    for (const QPointer<QDockWidget>& dock_pointer: m_docks)
    {
        QDockWidget* dock_widget = dock_pointer.data();
        if (dock_widget != nullptr)
        {
            toggle_actions.append(dock_widget->toggleViewAction());
        }
    }

    return toggle_actions;
}

/**
 * @brief Temporarily hides or restores all registered docks.
 * @param suspended True to capture the current layout and hide all docks; false to restore the
 * captured layout.
 */
auto DockController::set_docks_suspended(bool suspended) -> void
{
    if (m_docks_suspended != suspended)
    {
        if (suspended)
        {
            if (m_main_window != nullptr)
            {
                m_suspended_layout_state = m_main_window->saveState();
            }

            m_docks_suspended = true;
            for (const QPointer<QDockWidget>& dock_pointer: std::as_const(m_docks))
            {
                QDockWidget* dock_widget = dock_pointer.data();
                if (dock_widget != nullptr)
                {
                    dock_widget->toggleViewAction()->setEnabled(false);
                    dock_widget->setVisible(false);
                }
            }
        }
        else
        {
            if (m_main_window != nullptr && !m_suspended_layout_state.isEmpty())
            {
                m_main_window->restoreState(m_suspended_layout_state);
            }

            for (const QPointer<QDockWidget>& dock_pointer: std::as_const(m_docks))
            {
                QDockWidget* dock_widget = dock_pointer.data();
                if (dock_widget != nullptr)
                {
                    dock_widget->toggleViewAction()->setEnabled(true);
                }
            }

            m_docks_suspended = false;
        }
    }
}
