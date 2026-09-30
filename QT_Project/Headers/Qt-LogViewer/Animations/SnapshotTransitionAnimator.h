#pragma once

#include <QEasingCurve>
#include <QObject>
#include <QPoint>
#include <QPointer>
#include <functional>

class QLabel;
class QPropertyAnimation;
class QWidget;

/**
 * @file SnapshotTransitionAnimator.h
 * @brief Declares reusable snapshot-based QWidget transitions.
 */

/**
 * @enum WidgetTransitionEffect
 * @brief Selects how outgoing and incoming widget snapshots are animated.
 */
enum class WidgetTransitionEffect
{
    CrossFade,    ///< Crossfades both snapshots without movement.
    Slide,        ///< Slides both snapshots across the complete surface.
    FadeAndSlide  ///< Crossfades while applying a short directional movement.
};

/**
 * @enum WidgetTransitionDirection
 * @brief Selects the direction in which widget snapshots move.
 */
enum class WidgetTransitionDirection
{
    Left,   ///< Moves snapshots from right to left.
    Right,  ///< Moves snapshots from left to right.
    Up,     ///< Moves snapshots from bottom to top.
    Down    ///< Moves snapshots from top to bottom.
};

/**
 * @struct WidgetTransitionConfiguration
 * @brief Groups all externally configurable snapshot-transition settings.
 */
struct WidgetTransitionConfiguration {
        /** Visual effect applied to the outgoing and incoming snapshots. */
        WidgetTransitionEffect effect{WidgetTransitionEffect::FadeAndSlide};

        /** Direction used by slide-based effects. */
        WidgetTransitionDirection direction{WidgetTransitionDirection::Up};

        /** Timing curve applied to normalized transition progress. */
        QEasingCurve easing_curve{QEasingCurve::InOutQuad};

        /** Complete animation duration in milliseconds; zero disables animation. */
        int duration_ms{300};

        /** Movement distance in pixels used by FadeAndSlide. */
        int motion_distance{10};
};

/**
 * @class SnapshotTransitionAnimator
 * @brief Animates any QWidget surface around a synchronous visual state change.
 *
 * The animator captures the supplied surface before and after invoking the target-state callback.
 * Temporary overlays hide intermediate layouts and allow the same implementation to animate
 * stacked pages, complete table refreshes, dialog pages, charts, or other composite widgets.
 */
class SnapshotTransitionAnimator final: public QObject
{
        Q_OBJECT
        Q_PROPERTY(
            qreal transition_progress READ get_transition_progress WRITE set_transition_progress)

    public:
        /**
         * @brief Constructs a snapshot transition animator.
         * @param parent Optional QObject parent.
         */
        explicit SnapshotTransitionAnimator(QObject* parent = nullptr);

        /** @brief Removes temporary overlays before the animator is destroyed. */
        ~SnapshotTransitionAnimator() override;

        /**
         * @brief Animates a synchronous visual state change on one widget surface.
         * @param surface Non-owning widget surface captured before and after the update.
         * @param apply_target_state Callback that synchronously applies the target visual state.
         *
         * A running transition is completed before the new one starts. Null or hidden surfaces and
         * a zero duration apply the callback immediately without creating overlays.
         */
        auto transition(QWidget* surface, const std::function<void()>& apply_target_state) -> void;

        /**
         * @brief Completes an active transition and removes its overlays.
         *
         * Calling this method while idle has no effect.
         */
        auto finish() -> void;

        /**
         * @brief Applies all animation settings in one operation.
         * @param configuration Requested effect, timing, easing, direction, and movement.
         *
         * Negative durations and movement distances are clamped to zero. Requests made during an
         * active transition are ignored to keep the current frame sequence stable.
         */
        auto set_configuration(const WidgetTransitionConfiguration& configuration) -> void;

        /**
         * @brief Returns the effective transition configuration.
         * @return Current effect, timing, easing, direction, and clamped movement settings.
         */
        [[nodiscard]] auto get_configuration() const -> WidgetTransitionConfiguration;

        /**
         * @brief Reports whether a snapshot animation is active.
         * @return True while the configured transition animation is running; otherwise false.
         */
        [[nodiscard]] auto is_running() const -> bool;

    signals:
        /** @brief Emitted after the target snapshot has replaced all temporary overlays. */
        void finished();

    protected:
        /**
         * @brief Keeps overlays aligned and completes transitions when the surface is hidden.
         * @param watched Object receiving the event.
         * @param event Delivered event.
         * @return Result produced by the base QObject event filter.
         */
        auto eventFilter(QObject* watched, QEvent* event) -> bool override;

    private:
        /**
         * @brief Returns the normalized progress of the active transition.
         * @return Progress in the inclusive range from zero to one.
         */
        [[nodiscard]] auto get_transition_progress() const -> qreal;

        /**
         * @brief Applies opacity and geometry for one animation frame.
         * @param progress Requested normalized progress value.
         */
        auto set_transition_progress(qreal progress) -> void;

        /** @brief Updates overlay opacity and geometry from the current progress. */
        auto update_overlay_presentation() -> void;

        /**
         * @brief Calculates a directional movement vector.
         * @param distance Non-negative movement distance in pixels.
         * @return Signed movement vector for the configured direction.
         */
        [[nodiscard]] auto get_motion_delta(int distance) const -> QPoint;

        /**
         * @brief Removes every temporary animation object and optionally reports completion.
         * @param notify Whether to emit finished().
         */
        auto complete_transition(bool notify) -> void;

        QPointer<QWidget> m_surface;
        QPointer<QLabel> m_background_overlay;
        QPointer<QLabel> m_outgoing_overlay;
        QPointer<QLabel> m_incoming_overlay;
        QPointer<QPropertyAnimation> m_animation;
        WidgetTransitionConfiguration m_configuration;
        qreal m_transition_progress{0.0};
};
