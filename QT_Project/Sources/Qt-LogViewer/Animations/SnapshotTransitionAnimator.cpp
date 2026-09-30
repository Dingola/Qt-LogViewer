/**
 * @file SnapshotTransitionAnimator.cpp
 * @brief Implements reusable snapshot-based QWidget transitions.
 */

#include "Qt-LogViewer/Animations/SnapshotTransitionAnimator.h"

#include <QAbstractAnimation>
#include <QApplication>
#include <QEvent>
#include <QGraphicsOpacityEffect>
#include <QLabel>
#include <QLayout>
#include <QPalette>
#include <QPixmap>
#include <QPropertyAnimation>
#include <QWidget>
#include <algorithm>

/**
 * @brief Constructs a snapshot transition animator.
 * @param parent Optional QObject parent.
 */
SnapshotTransitionAnimator::SnapshotTransitionAnimator(QObject* parent): QObject(parent) {}

/** @brief Removes temporary overlays before the animator is destroyed. */
SnapshotTransitionAnimator::~SnapshotTransitionAnimator()
{
    complete_transition(false);
}

/**
 * @brief Animates a synchronous visual state change on one widget surface.
 * @param surface Non-owning widget surface captured before and after the update.
 * @param apply_target_state Callback that synchronously applies the target visual state.
 */
auto SnapshotTransitionAnimator::transition(QWidget* surface,
                                            const std::function<void()>& apply_target_state) -> void
{
    if (is_running())
    {
        complete_transition(true);
    }

    const bool can_animate =
        surface != nullptr && surface->isVisible() && m_configuration.duration_ms > 0;

    if (can_animate)
    {
        m_surface = surface;
        m_surface->installEventFilter(this);
        const QPixmap outgoing_pixmap = m_surface->grab();

        m_background_overlay = new QLabel(m_surface);
        QPixmap background_pixmap(m_surface->size());
        QColor background_color = m_surface->palette().color(m_surface->backgroundRole());
        background_color.setAlpha(255);
        background_pixmap.fill(background_color);
        m_background_overlay->setPixmap(background_pixmap);
        m_background_overlay->setScaledContents(true);
        m_background_overlay->setAttribute(Qt::WA_OpaquePaintEvent);
        m_background_overlay->setAttribute(Qt::WA_TransparentForMouseEvents);
        m_background_overlay->setGeometry(m_surface->rect());
        m_background_overlay->show();

        m_outgoing_overlay = new QLabel(m_surface);
        m_outgoing_overlay->setScaledContents(true);
        m_outgoing_overlay->setPixmap(outgoing_pixmap);
        m_outgoing_overlay->setAttribute(Qt::WA_TransparentForMouseEvents);
        m_outgoing_overlay->setGraphicsEffect(new QGraphicsOpacityEffect(m_outgoing_overlay));
        m_outgoing_overlay->setGeometry(m_surface->rect());
        m_outgoing_overlay->show();

        if (apply_target_state)
        {
            apply_target_state();
        }

        if (!m_surface.isNull())
        {
            if (m_surface->layout() != nullptr)
            {
                m_surface->layout()->activate();
            }
            QApplication::sendPostedEvents(nullptr, QEvent::LayoutRequest);

            m_outgoing_overlay->hide();
            m_background_overlay->hide();
            const QPixmap incoming_pixmap = m_surface->grab();
            m_background_overlay->show();
            m_outgoing_overlay->show();

            m_incoming_overlay = new QLabel(m_surface);
            m_incoming_overlay->setScaledContents(true);
            m_incoming_overlay->setPixmap(incoming_pixmap);
            m_incoming_overlay->setAttribute(Qt::WA_TransparentForMouseEvents);
            m_incoming_overlay->setGraphicsEffect(new QGraphicsOpacityEffect(m_incoming_overlay));

            m_transition_progress = 0.0;
            update_overlay_presentation();
            m_incoming_overlay->show();
            m_background_overlay->raise();
            m_outgoing_overlay->raise();
            m_incoming_overlay->raise();

            m_animation = new QPropertyAnimation(this, "transition_progress", this);
            m_animation->setDuration(m_configuration.duration_ms);
            m_animation->setStartValue(0.0);
            m_animation->setEndValue(1.0);
            m_animation->setEasingCurve(m_configuration.easing_curve);
            connect(m_animation, &QPropertyAnimation::finished, this,
                    [this] { complete_transition(true); });
            m_animation->start();
        }
        else
        {
            complete_transition(true);
        }
    }
    else
    {
        if (apply_target_state)
        {
            apply_target_state();
        }
        emit finished();
    }
}

/** @brief Completes an active transition and removes its overlays. */
auto SnapshotTransitionAnimator::finish() -> void
{
    if (is_running())
    {
        complete_transition(true);
    }
}

/**
 * @brief Applies all animation settings in one operation.
 * @param configuration Requested effect, timing, easing, direction, and movement.
 */
auto SnapshotTransitionAnimator::set_configuration(
    const WidgetTransitionConfiguration& configuration) -> void
{
    if (!is_running())
    {
        m_configuration = configuration;
        m_configuration.duration_ms = std::max(0, configuration.duration_ms);
        m_configuration.motion_distance = std::max(0, configuration.motion_distance);
    }
}

/**
 * @brief Returns the effective transition configuration.
 * @return Current effect, timing, easing, direction, and clamped movement settings.
 */
auto SnapshotTransitionAnimator::get_configuration() const -> WidgetTransitionConfiguration
{
    return m_configuration;
}

/**
 * @brief Reports whether a snapshot animation is active.
 * @return True while the configured transition animation is running; otherwise false.
 */
auto SnapshotTransitionAnimator::is_running() const -> bool
{
    const bool running =
        !m_animation.isNull() && m_animation->state() == QAbstractAnimation::Running;
    return running;
}

/**
 * @brief Keeps overlays aligned and completes transitions when the surface becomes unavailable.
 * @param watched Object receiving the event.
 * @param event Delivered event.
 * @return Result produced by the base QObject event filter.
 */
auto SnapshotTransitionAnimator::eventFilter(QObject* watched, QEvent* event) -> bool
{
    const bool surface_event = watched == m_surface.data();
    const bool surface_hidden = surface_event && event->type() == QEvent::Hide;
    const bool window_minimized = surface_event && event->type() == QEvent::WindowStateChange &&
                                  m_surface->window()->windowState().testFlag(Qt::WindowMinimized);

    if (surface_hidden || window_minimized)
    {
        complete_transition(true);
    }
    else if (surface_event &&
             (event->type() == QEvent::Resize || event->type() == QEvent::LayoutRequest))
    {
        update_overlay_presentation();
    }

    const bool handled = QObject::eventFilter(watched, event);
    return handled;
}

/**
 * @brief Returns the normalized progress of the active transition.
 * @return Progress in the inclusive range from zero to one.
 */
auto SnapshotTransitionAnimator::get_transition_progress() const -> qreal
{
    return m_transition_progress;
}

/**
 * @brief Applies opacity and geometry for one animation frame.
 * @param progress Requested normalized progress value.
 */
auto SnapshotTransitionAnimator::set_transition_progress(qreal progress) -> void
{
    m_transition_progress = std::clamp(progress, 0.0, 1.0);
    update_overlay_presentation();
}

/** @brief Updates overlay opacity and geometry from the current progress. */
auto SnapshotTransitionAnimator::update_overlay_presentation() -> void
{
    if (!m_surface.isNull())
    {
        const QRect surface_geometry = m_surface->rect();
        QPoint outgoing_offset;
        QPoint incoming_offset;
        qreal outgoing_opacity = 1.0;
        qreal incoming_opacity = 1.0;

        switch (m_configuration.effect)
        {
        case WidgetTransitionEffect::CrossFade:
            outgoing_opacity = 1.0 - m_transition_progress;
            incoming_opacity = m_transition_progress;
            break;

        case WidgetTransitionEffect::Slide: {
            const bool horizontal = m_configuration.direction == WidgetTransitionDirection::Left ||
                                    m_configuration.direction == WidgetTransitionDirection::Right;
            const int distance = horizontal ? surface_geometry.width() : surface_geometry.height();
            const QPoint delta = get_motion_delta(distance);
            outgoing_offset = QPoint(qRound(static_cast<qreal>(delta.x()) * m_transition_progress),
                                     qRound(static_cast<qreal>(delta.y()) * m_transition_progress));
            incoming_offset =
                QPoint(qRound(static_cast<qreal>(delta.x()) * (m_transition_progress - 1.0)),
                       qRound(static_cast<qreal>(delta.y()) * (m_transition_progress - 1.0)));
            break;
        }

        case WidgetTransitionEffect::FadeAndSlide: {
            const QPoint delta = get_motion_delta(m_configuration.motion_distance);
            outgoing_opacity = 1.0 - m_transition_progress;
            incoming_opacity = m_transition_progress;
            outgoing_offset = QPoint(qRound(static_cast<qreal>(delta.x()) * m_transition_progress),
                                     qRound(static_cast<qreal>(delta.y()) * m_transition_progress));
            incoming_offset =
                QPoint(qRound(static_cast<qreal>(delta.x()) * (m_transition_progress - 1.0)),
                       qRound(static_cast<qreal>(delta.y()) * (m_transition_progress - 1.0)));
            break;
        }
        }

        if (!m_background_overlay.isNull())
        {
            m_background_overlay->setGeometry(surface_geometry);
        }

        if (!m_outgoing_overlay.isNull())
        {
            QRect outgoing_geometry = surface_geometry;
            outgoing_geometry.translate(outgoing_offset);
            m_outgoing_overlay->setGeometry(outgoing_geometry);
            auto* opacity =
                qobject_cast<QGraphicsOpacityEffect*>(m_outgoing_overlay->graphicsEffect());
            if (opacity != nullptr)
            {
                opacity->setOpacity(outgoing_opacity);
            }
        }

        if (!m_incoming_overlay.isNull())
        {
            QRect incoming_geometry = surface_geometry;
            incoming_geometry.translate(incoming_offset);
            m_incoming_overlay->setGeometry(incoming_geometry);
            auto* opacity =
                qobject_cast<QGraphicsOpacityEffect*>(m_incoming_overlay->graphicsEffect());
            if (opacity != nullptr)
            {
                opacity->setOpacity(incoming_opacity);
            }
        }
    }
}

/**
 * @brief Calculates a directional movement vector.
 * @param distance Non-negative movement distance in pixels.
 * @return Signed movement vector for the configured direction.
 */
auto SnapshotTransitionAnimator::get_motion_delta(int distance) const -> QPoint
{
    QPoint delta;

    switch (m_configuration.direction)
    {
    case WidgetTransitionDirection::Left:
        delta = QPoint(-distance, 0);
        break;

    case WidgetTransitionDirection::Right:
        delta = QPoint(distance, 0);
        break;

    case WidgetTransitionDirection::Up:
        delta = QPoint(0, -distance);
        break;

    case WidgetTransitionDirection::Down:
        delta = QPoint(0, distance);
        break;
    }

    return delta;
}

/**
 * @brief Removes every temporary animation object and optionally reports completion.
 * @param notify Whether to emit finished().
 */
auto SnapshotTransitionAnimator::complete_transition(bool notify) -> void
{
    if (!m_animation.isNull())
    {
        m_animation->stop();
        m_animation->deleteLater();
        m_animation = nullptr;
    }

    if (!m_background_overlay.isNull())
    {
        m_background_overlay->hide();
        m_background_overlay->deleteLater();
        m_background_overlay = nullptr;
    }

    if (!m_outgoing_overlay.isNull())
    {
        m_outgoing_overlay->hide();
        m_outgoing_overlay->deleteLater();
        m_outgoing_overlay = nullptr;
    }

    if (!m_incoming_overlay.isNull())
    {
        m_incoming_overlay->hide();
        m_incoming_overlay->deleteLater();
        m_incoming_overlay = nullptr;
    }

    if (!m_surface.isNull())
    {
        m_surface->removeEventFilter(this);
        m_surface = nullptr;
    }

    m_transition_progress = 0.0;

    if (notify)
    {
        emit finished();
    }
}
