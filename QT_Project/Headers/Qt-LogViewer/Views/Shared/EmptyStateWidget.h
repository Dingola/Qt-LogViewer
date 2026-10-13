#pragma once

#include <QHash>
#include <QIcon>
#include <QWidget>

class QAction;
class QBoxLayout;
class QLabel;
class QPushButton;
class QVBoxLayout;

/**
 * @file EmptyStateWidget.h
 * @brief Declares a reusable presentation widget for empty content areas.
 */

/**
 * @class EmptyStateWidget
 * @brief Presents explanatory text, optional drop guidance, and context-owned
 * actions.
 *
 * The widget deliberately contains no application or mime-data logic. Hosts
 * decide which state is active and provide domain actions through QAction
 * instances.
 */
class EmptyStateWidget: public QWidget
{
        Q_OBJECT

    public:
        /** @brief Controls the visual spacing of the empty-state content. */
        enum class Density
        {
            Regular,
            Compact
        };

        /** @brief Describes the visual state of an optional drop target. */
        enum class DropState
        {
            Hidden,
            Available,
            Accepting,
            Rejected
        };

        /** @brief Maps context actions onto the application's semantic button
         * hierarchy. */
        enum class ActionRole
        {
            Primary,
            Secondary,
            Ghost,
            Danger
        };

        /**
         * @brief Constructs an empty-state widget.
         * @param parent Parent widget.
         */
        explicit EmptyStateWidget(QWidget* parent = nullptr);

        /**
         * @brief Returns no preferred extent so the presentation cannot resize its host.
         * @return An empty preferred size.
         */
        [[nodiscard]] auto sizeHint() const -> QSize override;

        /**
         * @brief Returns no minimum extent so persisted host sizes remain authoritative.
         * @return An empty minimum size.
         */
        [[nodiscard]] auto minimumSizeHint() const -> QSize override;

        /**
         * @brief Sets the optional leading icon.
         * @param icon Icon to display, or a null icon to hide it.
         */
        auto set_icon(const QIcon& icon) -> void;

        /**
         * @brief Sets the concise state heading.
         * @param title Heading shown above the description.
         */
        auto set_title(const QString& title) -> void;

        /**
         * @brief Sets the explanatory supporting text.
         * @param description Description shown below the heading.
         */
        auto set_description(const QString& description) -> void;

        /**
         * @brief Selects regular or compact spacing.
         * @param density Desired presentation density.
         */
        auto set_density(Density density) -> void;

        /**
         * @brief Aligns the complete content group inside the available area.
         * @param alignment Horizontal and vertical content alignment.
         */
        auto set_alignment(Qt::Alignment alignment) -> void;

        /**
         * @brief Sets the optional drop-target presentation.
         * @param state Visual drop state selected by the host.
         */
        auto set_drop_state(DropState state) -> void;

        /**
         * @brief Sets the drop guidance text.
         * @param text Text displayed inside the drop hint.
         */
        auto set_drop_text(const QString& text) -> void;

        /**
         * @brief Controls whether a hidden drop hint keeps its layout height.
         * @param reserved True to preserve the hint's space while hidden.
         */
        auto set_drop_space_reserved(bool reserved) -> void;

        /**
         * @brief Adds a context-owned action and mirrors its state on a button.
         * @param action Action to expose without taking ownership.
         * @param role Semantic visual role for the generated button.
         */
        auto add_action(QAction* action, ActionRole role = ActionRole::Secondary) -> void;

        /**
         * @brief Removes one action button without deleting the action.
         * @param action Action whose button is removed.
         */
        auto remove_action(QAction* action) -> void;

        /** @brief Removes all action buttons without deleting their actions. */
        auto clear_actions() -> void;

    private:
        /**
         * @brief Mirrors the current QAction presentation onto its button.
         * @param action Source action.
         * @param button Destination button.
         */
        auto synchronize_action_button(QAction* action, QPushButton* button) -> void;

        /**
         * @brief Converts an action role to the shared stylesheet property value.
         * @param role Semantic action role.
         * @return Stylesheet role name.
         */
        [[nodiscard]] static auto action_role_name(ActionRole role) -> QString;

        /** @brief Applies density-dependent margins and spacing. */
        auto update_density() -> void;

        /** @brief Applies the selected horizontal and vertical content alignment. */
        auto update_alignment() -> void;

        QLabel* m_icon_label{nullptr};
        QLabel* m_title_label{nullptr};
        QLabel* m_description_label{nullptr};
        QLabel* m_drop_label{nullptr};
        QWidget* m_action_container{nullptr};
        QVBoxLayout* m_main_layout{nullptr};
        QBoxLayout* m_action_layout{nullptr};
        QHash<QAction*, QPushButton*> m_action_buttons;
        Density m_density{Density::Regular};
        Qt::Alignment m_alignment{Qt::AlignCenter};
};
