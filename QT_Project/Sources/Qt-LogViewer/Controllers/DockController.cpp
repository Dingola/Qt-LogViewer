/**
 * @file DockController.cpp
 * @brief Implements DockController for registered dock widgets and layout suspension.
 */

#include "Qt-LogViewer/Controllers/DockController.h"

#include <QAction>
#include <QDockWidget>
#include <QEvent>
#include <QLayout>
#include <QMainWindow>
#include <QScopedValueRollback>
#include <QTimer>
#include <utility>

/**
 * @brief Constructs a DockController.
 * @param main_window Window whose dock layout is managed.
 * @param parent Optional QObject parent.
 */
DockController::DockController(QMainWindow* main_window, QObject* parent)
    : QObject(parent),
      m_main_window(main_window),
      m_observed_main_window_size(main_window != nullptr ? main_window->size() : QSize())
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
        m_preferred_sizes.insert(dock_widget, dock_widget->size());
        dock_widget->installEventFilter(this);

        connect(dock_widget, &QObject::destroyed, this,
                [this, dock_widget] { m_preferred_sizes.remove(dock_widget); });

        if (m_docks_suspended)
        {
            dock_widget->toggleViewAction()->setEnabled(false);
            dock_widget->setVisible(false);
        }
    }
}

/**
 * @brief Tracks dock sizes selected through the dock splitters.
 * @param watched Object receiving the event.
 * @param event Event being delivered.
 * @return The base QObject event-filter result.
 */
bool DockController::eventFilter(QObject* watched, QEvent* event)
{
    const bool can_track_size = m_main_window != nullptr && !m_applying_sizes &&
                                !m_main_window_resize_active && !m_docks_suspended;
    if (can_track_size && event->type() == QEvent::Resize &&
        m_main_window->size() == m_observed_main_window_size)
    {
        auto* dock = qobject_cast<QDockWidget*>(watched);
        if (dock != nullptr && dock->isVisible() && !dock->isFloating())
        {
            m_preferred_sizes[dock] = dock->size();
        }
    }

    return QObject::eventFilter(watched, event);
}

/**
 * @brief Restores preferred dock extents after QMainWindow handled a resize event.
 * @param old_size Previous QMainWindow size.
 * @param new_size New QMainWindow size.
 */
auto DockController::handle_main_window_resize(const QSize& old_size, const QSize& new_size) -> void
{
    if (m_main_window != nullptr)
    {
        m_main_window_resize_active = true;
        const quint64 generation = ++m_resize_generation;

        if (!m_docks_suspended && old_size.isValid())
        {
            QScopedValueRollback<bool> applying(m_applying_sizes, true);
            const bool changed = restore_preferred_sizes(new_size.width() > old_size.width(),
                                                         new_size.height() > old_size.height());
            if (changed)
            {
                m_main_window->layout()->activate();
            }
        }

        m_observed_main_window_size = m_main_window->size();
        QTimer::singleShot(0, this, [this, generation] {
            if (generation == m_resize_generation)
            {
                m_main_window_resize_active = false;
            }
        });
    }
}

/**
 * @brief Captures the currently visible dock layout as the preferred layout.
 */
auto DockController::capture_current_sizes() -> void
{
    for (const auto& pointer: std::as_const(m_docks))
    {
        QDockWidget* dock = pointer.data();
        if (dock != nullptr && dock->isVisible() && !dock->isFloating())
        {
            m_preferred_sizes[dock] = dock->size();
        }
    }

    if (m_main_window != nullptr)
    {
        m_observed_main_window_size = m_main_window->size();
    }
}

/**
 * @brief Requests preferred extents without constraining the layout's minimum sizes.
 * @param restore_width True to restore the preferred width of each dock.
 * @param restore_height True to restore the preferred height of each dock.
 * @return True if any dock extents were applied; false if no matching docks were visible.
 */
auto DockController::restore_preferred_sizes(bool restore_width, bool restore_height) -> bool
{
    bool changed = false;
    const QList<Qt::DockWidgetArea> areas = {Qt::LeftDockWidgetArea, Qt::RightDockWidgetArea,
                                             Qt::TopDockWidgetArea, Qt::BottomDockWidgetArea};

    for (Qt::DockWidgetArea area: areas)
    {
        QList<QDockWidget*> area_docks;
        QList<int> widths;
        QList<int> heights;

        for (const auto& pointer: std::as_const(m_docks))
        {
            QDockWidget* dock = pointer.data();
            const bool belongs_to_area = dock != nullptr && dock->isVisible() &&
                                         !dock->isFloating() &&
                                         m_main_window->dockWidgetArea(dock) == area;
            if (belongs_to_area)
            {
                const QSize preferred = m_preferred_sizes.value(dock, dock->size());
                area_docks.append(dock);
                widths.append(preferred.width());
                heights.append(preferred.height());
            }
        }

        const bool side_area = area == Qt::LeftDockWidgetArea || area == Qt::RightDockWidgetArea;
        if (!area_docks.isEmpty() &&
            ((side_area && restore_width) || (!side_area && restore_height)))
        {
            m_main_window->resizeDocks(area_docks, side_area ? widths : heights,
                                       side_area ? Qt::Horizontal : Qt::Vertical);
            changed = true;
        }

        // Preserve the user-controlled split inside a dock area as well. A single dock follows
        // the available window extent on this axis and therefore needs no second request.
        if (area_docks.size() > 1 &&
            ((side_area && restore_height) || (!side_area && restore_width)))
        {
            m_main_window->resizeDocks(area_docks, side_area ? heights : widths,
                                       side_area ? Qt::Vertical : Qt::Horizontal);
            changed = true;
        }
    }

    return changed;
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
