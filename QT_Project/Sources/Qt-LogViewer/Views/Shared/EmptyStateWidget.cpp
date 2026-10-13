/**
 * @file EmptyStateWidget.cpp
 * @brief Implements the reusable empty-content presentation widget.
 */

#include "Qt-LogViewer/Views/Shared/EmptyStateWidget.h"

#include <QAction>
#include <QBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSizePolicy>
#include <QStyle>
#include <QVBoxLayout>

/**
 * @brief Constructs the empty-state presentation and its action container.
 * @param parent Parent widget.
 */
EmptyStateWidget::EmptyStateWidget(QWidget* parent)
    : QWidget(parent),
      m_icon_label(new QLabel(this)),
      m_title_label(new QLabel(this)),
      m_description_label(new QLabel(this)),
      m_drop_label(new QLabel(this)),
      m_action_container(new QWidget(this)),
      m_main_layout(new QVBoxLayout(this)),
      m_action_layout(new QBoxLayout(QBoxLayout::LeftToRight, m_action_container))
{
    setObjectName(QStringLiteral("emptyStateWidget"));
    setProperty("density", QStringLiteral("regular"));
    setAttribute(Qt::WA_StyledBackground, true);
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    m_icon_label->setObjectName(QStringLiteral("emptyStateIcon"));
    m_icon_label->setVisible(false);
    m_title_label->setObjectName(QStringLiteral("emptyStateTitle"));
    m_title_label->setWordWrap(true);
    m_title_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_description_label->setObjectName(QStringLiteral("emptyStateDescription"));
    m_description_label->setWordWrap(true);
    m_description_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_drop_label->setObjectName(QStringLiteral("emptyStateDropHint"));
    m_drop_label->setWordWrap(true);
    m_drop_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_drop_label->setVisible(false);
    m_action_container->setObjectName(QStringLiteral("emptyStateActions"));
    m_action_layout->setContentsMargins(0, 0, 0, 0);
    m_action_layout->setSpacing(8);

    m_main_layout->addStretch(1);
    m_main_layout->addWidget(m_icon_label);
    m_main_layout->addWidget(m_title_label);
    m_main_layout->addWidget(m_description_label);
    m_main_layout->addWidget(m_drop_label);
    m_main_layout->addWidget(m_action_container);
    m_main_layout->addStretch(1);

    update_density();
    update_alignment();
}

/**
 * @brief Returns no preferred extent so the empty presentation cannot resize its host.
 * @return An empty preferred size.
 */
auto EmptyStateWidget::sizeHint() const -> QSize
{
    return QSize();
}

/**
 * @brief Returns no minimum extent so persisted host sizes remain authoritative.
 * @return An empty minimum size.
 */
auto EmptyStateWidget::minimumSizeHint() const -> QSize
{
    return QSize();
}

/**
 * @brief Sets or hides the optional leading icon.
 * @param icon Icon to display.
 */
auto EmptyStateWidget::set_icon(const QIcon& icon) -> void
{
    m_icon_label->setPixmap(icon.pixmap(32, 32));
    m_icon_label->setVisible(!icon.isNull());
}

/**
 * @brief Updates the state heading.
 * @param title Heading text.
 */
auto EmptyStateWidget::set_title(const QString& title) -> void
{
    m_title_label->setText(title);
    m_title_label->setVisible(!title.isEmpty());
}

/**
 * @brief Updates the explanatory supporting text.
 * @param description Description text.
 */
auto EmptyStateWidget::set_description(const QString& description) -> void
{
    m_description_label->setText(description);
    m_description_label->setVisible(!description.isEmpty());
}

/**
 * @brief Changes the presentation density.
 * @param density Desired density.
 */
auto EmptyStateWidget::set_density(Density density) -> void
{
    m_density = density;
    setProperty("density", density == Density::Compact ? QStringLiteral("compact")
                                                       : QStringLiteral("regular"));
    style()->unpolish(this);
    style()->polish(this);
    update_density();
}

/**
 * @brief Changes the horizontal and vertical content alignment.
 * @param alignment Complete content alignment.
 */
auto EmptyStateWidget::set_alignment(Qt::Alignment alignment) -> void
{
    m_alignment = alignment;
    update_alignment();
}

/**
 * @brief Updates the host-controlled drop-target appearance.
 * @param state Visual drop state.
 */
auto EmptyStateWidget::set_drop_state(DropState state) -> void
{
    QString state_name;
    switch (state)
    {
    case DropState::Available:
        state_name = QStringLiteral("available");
        break;
    case DropState::Accepting:
        state_name = QStringLiteral("accepting");
        break;
    case DropState::Rejected:
        state_name = QStringLiteral("rejected");
        break;
    case DropState::Hidden:
        state_name = QStringLiteral("hidden");
        break;
    }

    m_drop_label->setProperty("dropState", state_name);
    m_drop_label->setVisible(state != DropState::Hidden);
    m_drop_label->style()->unpolish(m_drop_label);
    m_drop_label->style()->polish(m_drop_label);
}

/**
 * @brief Updates the optional drop guidance.
 * @param text Guidance text.
 */
auto EmptyStateWidget::set_drop_text(const QString& text) -> void
{
    m_drop_label->setText(text);
}

/**
 * @brief Controls whether hiding the drop hint changes the content block
 * height.
 * @param reserved True to preserve the hidden hint's layout space.
 */
auto EmptyStateWidget::set_drop_space_reserved(bool reserved) -> void
{
    QSizePolicy size_policy = m_drop_label->sizePolicy();
    size_policy.setRetainSizeWhenHidden(reserved);
    m_drop_label->setSizePolicy(size_policy);
}

/**
 * @brief Adds an action-backed button while leaving action ownership with the
 * caller.
 * @param action Action to expose.
 * @param role Semantic button role.
 */
auto EmptyStateWidget::add_action(QAction* action, ActionRole role) -> void
{
    if (action != nullptr && !m_action_buttons.contains(action))
    {
        auto* button = new QPushButton(m_action_container);
        button->setProperty("actionRole", action_role_name(role));
        m_action_buttons.insert(action, button);
        m_action_layout->addWidget(button);
        synchronize_action_button(action, button);

        connect(button, &QPushButton::clicked, action, &QAction::trigger);
        connect(action, &QAction::changed, this,
                [this, action, button] { synchronize_action_button(action, button); });
        connect(action, &QObject::destroyed, this, [this, action] { remove_action(action); });
    }
}

/**
 * @brief Removes an action-backed button without deleting the action.
 * @param action Action whose button is removed.
 */
auto EmptyStateWidget::remove_action(QAction* action) -> void
{
    QPushButton* button = m_action_buttons.take(action);
    if (button != nullptr)
    {
        disconnect(action, nullptr, this, nullptr);
        m_action_layout->removeWidget(button);
        button->deleteLater();
    }
}

/** @brief Removes every generated button while preserving the source actions.
 */
auto EmptyStateWidget::clear_actions() -> void
{
    const QList<QAction*> actions = m_action_buttons.keys();
    for (QAction* action: actions)
    {
        remove_action(action);
    }
}

/**
 * @brief Mirrors QAction properties onto a generated button.
 * @param action Source action.
 * @param button Destination button.
 */
auto EmptyStateWidget::synchronize_action_button(QAction* action, QPushButton* button) -> void
{
    if (action != nullptr && button != nullptr)
    {
        button->setText(action->text());
        button->setIcon(action->icon());
        button->setToolTip(action->toolTip());
        button->setStatusTip(action->statusTip());
        button->setShortcut(action->shortcut());
        button->setEnabled(action->isEnabled());
        button->setVisible(action->isVisible());
    }
}

/**
 * @brief Converts a semantic role to its shared stylesheet property.
 * @param role Semantic action role.
 * @return Stylesheet property value.
 */
auto EmptyStateWidget::action_role_name(ActionRole role) -> QString
{
    QString name;
    switch (role)
    {
    case ActionRole::Primary:
        name = QStringLiteral("primary");
        break;
    case ActionRole::Secondary:
        name = QStringLiteral("secondary");
        break;
    case ActionRole::Ghost:
        name = QStringLiteral("ghost");
        break;
    case ActionRole::Danger:
        name = QStringLiteral("danger");
        break;
    }
    return name;
}

/** @brief Applies margins and spacing for the selected density. */
auto EmptyStateWidget::update_density() -> void
{
    const bool compact = m_density == Density::Compact;
    const int horizontal_margin = compact ? 12 : 28;
    const int vertical_margin = compact ? 10 : 24;
    m_main_layout->setContentsMargins(horizontal_margin, vertical_margin, horizontal_margin,
                                      vertical_margin);
    m_main_layout->setSpacing(compact ? 5 : 9);
}

/** @brief Applies the selected horizontal and vertical alignment to the content
 * group. */
auto EmptyStateWidget::update_alignment() -> void
{
    Qt::Alignment horizontal_alignment = Qt::AlignHCenter;
    if (m_alignment.testFlag(Qt::AlignLeft))
    {
        horizontal_alignment = Qt::AlignLeft;
    }
    else if (m_alignment.testFlag(Qt::AlignRight))
    {
        horizontal_alignment = Qt::AlignRight;
    }

    const Qt::Alignment label_alignment = horizontal_alignment | Qt::AlignVCenter;
    m_icon_label->setAlignment(label_alignment);
    m_title_label->setAlignment(label_alignment);
    m_description_label->setAlignment(label_alignment);
    m_drop_label->setAlignment(label_alignment);
    m_main_layout->setAlignment(m_icon_label, horizontal_alignment);
    m_main_layout->setAlignment(m_action_container, horizontal_alignment);
    m_action_layout->setAlignment(horizontal_alignment);

    const bool align_top = m_alignment.testFlag(Qt::AlignTop);
    const bool align_bottom = m_alignment.testFlag(Qt::AlignBottom);
    m_main_layout->setStretch(0, align_top ? 0 : 1);
    m_main_layout->setStretch(6, align_bottom ? 0 : 1);
}
