#include "Qt-LogViewer/Views/Shared/DropOverlayWidget.h"

#include <QEvent>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPropertyAnimation>
#include <QSizePolicy>
#include <QStringList>
#include <QStyle>
#include <QVBoxLayout>

/**
 * @file DropOverlayWidget.cpp
 * @brief Implements a reusable presentation overlay for drag-and-drop
 * destinations.
 */

/**
 * @brief Constructs an initially hidden overlay covering its parent widget.
 * @param parent Surface covered by the overlay.
 */
DropOverlayWidget::DropOverlayWidget(QWidget* parent)
    : QWidget(parent),
      m_card(new QWidget(this)),
      m_icon_label(new QLabel(m_card)),
      m_title_label(new QLabel(m_card)),
      m_description_label(new QLabel(m_card)),
      m_detail_label(new QLabel(m_card)),
      m_opacity_effect(new QGraphicsOpacityEffect(this)),
      m_opacity_animation(new QPropertyAnimation(m_opacity_effect, "opacity", this))
{
    setObjectName(QStringLiteral("dropOverlayWidget"));
    setAttribute(Qt::WA_StyledBackground, true);
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    setFocusPolicy(Qt::NoFocus);
    setGraphicsEffect(m_opacity_effect);

    m_card->setObjectName(QStringLiteral("dropOverlayCard"));
    m_card->setAttribute(Qt::WA_StyledBackground, true);
    m_card->setMinimumWidth(460);
    m_card->setMaximumWidth(620);

    m_icon_label->setObjectName(QStringLiteral("dropOverlayIcon"));
    m_icon_label->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    m_icon_label->setFixedSize(40, 40);
    m_title_label->setObjectName(QStringLiteral("dropOverlayTitle"));
    m_title_label->setTextFormat(Qt::PlainText);
    m_title_label->setWordWrap(true);
    m_title_label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    m_description_label->setObjectName(QStringLiteral("dropOverlayDescription"));
    m_description_label->setTextFormat(Qt::PlainText);
    m_description_label->setWordWrap(true);
    m_description_label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    m_detail_label->setObjectName(QStringLiteral("dropOverlayDetail"));
    m_detail_label->setTextFormat(Qt::PlainText);
    m_detail_label->setWordWrap(true);
    m_detail_label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

    auto* text_layout = new QVBoxLayout();
    text_layout->setContentsMargins(0, 0, 0, 0);
    text_layout->setSpacing(7);
    text_layout->setSizeConstraint(QLayout::SetMinimumSize);
    text_layout->addWidget(m_title_label);
    text_layout->addWidget(m_description_label);
    text_layout->addWidget(m_detail_label);

    auto* card_layout = new QHBoxLayout(m_card);
    card_layout->setContentsMargins(22, 20, 22, 20);
    card_layout->setSpacing(16);
    card_layout->setSizeConstraint(QLayout::SetMinimumSize);
    card_layout->addWidget(m_icon_label, 0, Qt::AlignTop);
    card_layout->addLayout(text_layout, 1);

    auto* overlay_layout = new QVBoxLayout(this);
    overlay_layout->setContentsMargins(28, 28, 28, 28);
    overlay_layout->addStretch(1);
    overlay_layout->addWidget(m_card, 0, Qt::AlignCenter);
    overlay_layout->addStretch(1);

    m_opacity_effect->setOpacity(0.0);
    m_opacity_animation->setEasingCurve(QEasingCurve::InOutCubic);
    connect(m_opacity_animation, &QPropertyAnimation::finished, this, [this] {
        if (m_hiding)
        {
            hide();
        }
    });

    if (parentWidget() != nullptr)
    {
        parentWidget()->installEventFilter(this);
    }
    update_overlay_geometry();
    hide();
}

/**
 * @brief Applies icon, text, and semantic appearance atomically.
 * @param presentation Complete host-owned presentation state.
 */
auto DropOverlayWidget::set_presentation(const DropOverlayPresentation& presentation) -> void
{
    QPixmap icon_pixmap = presentation.icon.pixmap(32, 32);
    if (!icon_pixmap.isNull() && presentation.icon_tint.isValid())
    {
        QPainter painter(&icon_pixmap);
        painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        painter.fillRect(icon_pixmap.rect(), presentation.icon_tint);
    }
    m_icon_label->setPixmap(icon_pixmap);
    m_icon_label->setVisible(!icon_pixmap.isNull());
    m_title_label->setText(presentation.title);
    m_description_label->setText(presentation.description);
    m_description_label->setVisible(!presentation.description.isEmpty());
    m_detail_label->setText(presentation.detail);
    m_detail_label->setVisible(!presentation.detail.isEmpty());
    m_title_label->updateGeometry();
    m_description_label->updateGeometry();
    m_detail_label->updateGeometry();
    m_card->layout()->invalidate();
    m_card->layout()->activate();
    m_card->adjustSize();
    setAccessibleName(presentation.title);
    setAccessibleDescription(QStringList{presentation.description, presentation.detail}
                                 .join(QLatin1Char(' '))
                                 .trimmed());
    setProperty("dropState", visual_state_name(presentation.state));
    style()->unpolish(this);
    style()->polish(this);
    m_card->style()->unpolish(m_card);
    m_card->style()->polish(m_card);
}

/**
 * @brief Sets the fade duration used when visibility changes.
 * @param duration_ms Non-negative animation duration in milliseconds.
 */
auto DropOverlayWidget::set_animation_duration(int duration_ms) -> void
{
    m_animation_duration_ms = qMax(0, duration_ms);
}

/** @brief Raises and fades the overlay into view. */
auto DropOverlayWidget::show_overlay() -> void
{
    const bool start_fade = !isVisible() || m_hiding;
    m_hiding = false;
    update_overlay_geometry();
    raise();

    if (start_fade)
    {
        m_opacity_animation->stop();
        show();
        if (m_animation_duration_ms > 0)
        {
            m_opacity_animation->setDuration(m_animation_duration_ms);
            m_opacity_animation->setStartValue(m_opacity_effect->opacity());
            m_opacity_animation->setEndValue(1.0);
            m_opacity_animation->start();
        }
        else
        {
            m_opacity_effect->setOpacity(1.0);
        }
    }
}

/** @brief Fades the overlay out and hides it after the animation. */
auto DropOverlayWidget::hide_overlay() -> void
{
    if (isVisible())
    {
        m_hiding = true;
        m_opacity_animation->stop();
        if (m_animation_duration_ms > 0)
        {
            m_opacity_animation->setDuration(qMax(1, m_animation_duration_ms * 2 / 3));
            m_opacity_animation->setStartValue(m_opacity_effect->opacity());
            m_opacity_animation->setEndValue(0.0);
            m_opacity_animation->start();
        }
        else
        {
            m_opacity_effect->setOpacity(0.0);
            hide();
        }
    }
}

/**
 * @brief Reports whether the overlay is visible or currently fading out.
 * @return True while the overlay participates in presentation.
 */
auto DropOverlayWidget::is_overlay_visible() const -> bool
{
    const bool result = isVisible();
    return result;
}

/**
 * @brief Keeps the overlay aligned with its parent surface.
 * @param watched Object receiving the event.
 * @param event Delivered event.
 * @return Result produced by the base QWidget event filter.
 */
auto DropOverlayWidget::eventFilter(QObject* watched, QEvent* event) -> bool
{
    if (watched == parentWidget() &&
        (event->type() == QEvent::Resize || event->type() == QEvent::Show))
    {
        update_overlay_geometry();
    }
    const bool result = QWidget::eventFilter(watched, event);
    return result;
}

/** @brief Synchronizes the overlay geometry with its parent widget. */
auto DropOverlayWidget::update_overlay_geometry() -> void
{
    if (parentWidget() != nullptr)
    {
        setGeometry(parentWidget()->rect());
    }
}

/**
 * @brief Maps a semantic visual state to its stylesheet property.
 * @param state Semantic state selected by the host.
 * @return Stable stylesheet property value.
 */
auto DropOverlayWidget::visual_state_name(DropOverlayVisualState state) -> QString
{
    QString result;
    switch (state)
    {
    case DropOverlayVisualState::Neutral:
        result = QStringLiteral("neutral");
        break;
    case DropOverlayVisualState::Accepted:
        result = QStringLiteral("accepted");
        break;
    case DropOverlayVisualState::Warning:
        result = QStringLiteral("warning");
        break;
    case DropOverlayVisualState::Rejected:
        result = QStringLiteral("rejected");
        break;
    }
    return result;
}
