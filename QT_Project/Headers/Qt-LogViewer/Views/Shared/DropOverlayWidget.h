#pragma once

#include <QColor>
#include <QIcon>
#include <QString>
#include <QWidget>

class QEvent;
class QGraphicsOpacityEffect;
class QLabel;
class QPropertyAnimation;

/**
 * @file DropOverlayWidget.h
 * @brief Declares a reusable presentation overlay for drag-and-drop
 * destinations.
 */

/**
 * @brief Selects the semantic appearance of a drop overlay.
 */
enum class DropOverlayVisualState
{
    Neutral,
    Accepted,
    Warning,
    Rejected
};

/**
 * @brief Groups all content needed to present one drop destination.
 */
struct DropOverlayPresentation {
        /** @brief Optional icon representing the input or destination. */
        QIcon icon;
        /** @brief Optional tint for monochrome icons; invalid preserves original colors. */
        QColor icon_tint;
        /** @brief Primary action or payload summary. */
        QString title;
        /** @brief Explanation of the destination semantics. */
        QString description;
        /** @brief Optional secondary validation or quantity information. */
        QString detail;
        /** @brief Semantic visual state of the presentation. */
        DropOverlayVisualState state{DropOverlayVisualState::Neutral};
};

/**
 * @class DropOverlayWidget
 * @brief Displays host-provided drop guidance over any QWidget surface.
 *
 * The widget intentionally knows nothing about MIME formats, validation rules,
 * or application actions. Hosts provide a complete presentation and retain
 * ownership of drop behavior.
 */
class DropOverlayWidget: public QWidget
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs an initially hidden overlay covering its parent widget.
         * @param parent Surface covered by the overlay.
         */
        explicit DropOverlayWidget(QWidget* parent = nullptr);

        /**
         * @brief Applies icon, text, and semantic appearance atomically.
         * @param presentation Complete host-owned presentation state.
         */
        auto set_presentation(const DropOverlayPresentation& presentation) -> void;

        /**
         * @brief Sets the fade duration used when visibility changes.
         * @param duration_ms Non-negative animation duration in milliseconds.
         */
        auto set_animation_duration(int duration_ms) -> void;

        /** @brief Raises and fades the overlay into view. */
        auto show_overlay() -> void;

        /** @brief Fades the overlay out and hides it after the animation. */
        auto hide_overlay() -> void;

        /**
         * @brief Reports whether the overlay is visible or currently fading out.
         * @return True while the overlay participates in presentation.
         */
        [[nodiscard]] auto is_overlay_visible() const -> bool;

    protected:
        /**
         * @brief Keeps the overlay aligned with its parent surface.
         * @param watched Object receiving the event.
         * @param event Delivered event.
         * @return Result produced by the base QWidget event filter.
         */
        auto eventFilter(QObject* watched, QEvent* event) -> bool override;

    private:
        /** @brief Synchronizes the overlay geometry with its parent widget. */
        auto update_overlay_geometry() -> void;

        /**
         * @brief Maps a semantic visual state to its stylesheet property.
         * @param state Semantic state selected by the host.
         * @return Stable stylesheet property value.
         */
        [[nodiscard]] static auto visual_state_name(DropOverlayVisualState state) -> QString;

        QWidget* m_card{nullptr};
        QLabel* m_icon_label{nullptr};
        QLabel* m_title_label{nullptr};
        QLabel* m_description_label{nullptr};
        QLabel* m_detail_label{nullptr};
        QGraphicsOpacityEffect* m_opacity_effect{nullptr};
        QPropertyAnimation* m_opacity_animation{nullptr};
        int m_animation_duration_ms{260};
        bool m_hiding{false};
};
