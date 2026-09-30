/**
 * @file StackedWidgetTransition.cpp
 * @brief Implements the QStackedWidget adapter for generic snapshot transitions.
 */

#include "Qt-LogViewer/Animations/StackedWidgetTransition.h"

#include <QStackedWidget>

/**
 * @brief Constructs a transition adapter for one stacked widget.
 * @param stack_widget Non-owning stack whose pages are selected.
 * @param parent Optional QObject parent.
 */
StackedWidgetTransition::StackedWidgetTransition(QStackedWidget* stack_widget, QObject* parent)
    : QObject(parent),
      m_stack_widget(stack_widget),
      m_animator(new SnapshotTransitionAnimator(this))
{
    connect(m_animator, &SnapshotTransitionAnimator::finished, this, [this] {
        const int completed_index = m_target_index;
        m_target_index = -1;
        if (completed_index >= 0)
        {
            emit finished(completed_index);
        }
    });
}

/**
 * @brief Changes to a page using the configured snapshot animation.
 * @param index Target page index.
 */
auto StackedWidgetTransition::transition_to(int index) -> void
{
    const bool valid_index =
        !m_stack_widget.isNull() && index >= 0 && index < m_stack_widget->count();
    const bool same_pending_target = is_running() && m_target_index == index;
    const bool page_changed = valid_index && m_stack_widget->currentIndex() != index;

    if (page_changed && !same_pending_target)
    {
        if (is_running())
        {
            m_animator->finish();
        }

        m_target_index = index;
        QWidget* surface =
            !m_transition_surface.isNull() ? m_transition_surface.data() : m_stack_widget.data();
        m_animator->transition(surface, [this, index] {
            if (!m_stack_widget.isNull())
            {
                m_stack_widget->setCurrentIndex(index);
                emit target_page_activated(index);
            }
        });
    }
}

/**
 * @brief Sets the widget area captured during page transitions.
 * @param surface Non-owning surface, or nullptr to capture only the stacked widget.
 */
auto StackedWidgetTransition::set_transition_surface(QWidget* surface) -> void
{
    if (!is_running())
    {
        m_transition_surface = surface;
    }
}

/**
 * @brief Applies all generic snapshot animation settings.
 * @param configuration Requested effect, timing, easing, direction, and movement.
 */
auto StackedWidgetTransition::set_configuration(const WidgetTransitionConfiguration& configuration)
    -> void
{
    m_animator->set_configuration(configuration);
}

/**
 * @brief Returns the effective snapshot animation configuration.
 * @return Current effect, timing, easing, direction, and movement settings.
 */
auto StackedWidgetTransition::get_configuration() const -> WidgetTransitionConfiguration
{
    return m_animator->get_configuration();
}

/**
 * @brief Reports whether a page transition is active.
 * @return True while the adapter's snapshot animator is running; otherwise false.
 */
auto StackedWidgetTransition::is_running() const -> bool
{
    return m_animator->is_running();
}
