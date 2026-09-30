#pragma once

#include <QObject>
#include <QPointer>

#include "Qt-LogViewer/Animations/SnapshotTransitionAnimator.h"

class QStackedWidget;
class QWidget;

/**
 * @file StackedWidgetTransition.h
 * @brief Declares the QStackedWidget adapter for generic snapshot transitions.
 */

/**
 * @class StackedWidgetTransition
 * @brief Converts target page indices into generic snapshot transitions.
 *
 * This adapter owns the animator but not the supplied stack, its pages, or the optional
 * presentation surface. All effect rendering and timing remain in SnapshotTransitionAnimator.
 */
class StackedWidgetTransition final: public QObject
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs a transition adapter for one stacked widget.
         * @param stack_widget Non-owning stack whose pages are selected.
         * @param parent Optional QObject parent.
         */
        explicit StackedWidgetTransition(QStackedWidget* stack_widget, QObject* parent = nullptr);

        /**
         * @brief Changes to a page using the configured snapshot animation.
         * @param index Target page index.
         *
         * Invalid indices and repeated requests for the active target are ignored.
         */
        auto transition_to(int index) -> void;

        /**
         * @brief Sets the widget area captured during page transitions.
         * @param surface Non-owning surface, or nullptr to capture only the stacked widget.
         */
        auto set_transition_surface(QWidget* surface) -> void;

        /**
         * @brief Applies all generic snapshot animation settings.
         * @param configuration Requested effect, timing, easing, direction, and movement.
         */
        auto set_configuration(const WidgetTransitionConfiguration& configuration) -> void;

        /**
         * @brief Returns the effective snapshot animation configuration.
         * @return Current effect, timing, easing, direction, and movement settings.
         */
        [[nodiscard]] auto get_configuration() const -> WidgetTransitionConfiguration;

        /**
         * @brief Reports whether a page transition is active.
         * @return True while the adapter's snapshot animator is running; otherwise false.
         */
        [[nodiscard]] auto is_running() const -> bool;

    signals:
        /**
         * @brief Emitted synchronously after selecting the target page and before capturing it.
         * @param index Target page index.
         */
        void target_page_activated(int index);

        /**
         * @brief Emitted after the target page replaces all temporary overlays.
         * @param index Target page index.
         */
        void finished(int index);

    private:
        QPointer<QStackedWidget> m_stack_widget;
        QPointer<QWidget> m_transition_surface;
        SnapshotTransitionAnimator* m_animator{nullptr};
        int m_target_index{-1};
};
