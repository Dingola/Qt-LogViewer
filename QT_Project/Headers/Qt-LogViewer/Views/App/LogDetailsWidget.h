#pragma once

#include <QColor>
#include <QSize>
#include <QWidget>
#include <optional>

#include "Qt-LogViewer/Models/LogEntry.h"

class EmptyStateWidget;
class QEvent;
class QResizeEvent;
class QToolButton;

namespace Ui
{
class LogDetailsWidget;
}

/**
 * @file LogDetailsWidget.h
 * @brief Declares the structured inspector for one selected log entry.
 */

/**
 * @class LogDetailsWidget
 * @brief Presents parsed fields, source metadata, raw data, and copy actions for a log entry.
 */
class LogDetailsWidget: public QWidget
{
        Q_OBJECT
        Q_PROPERTY(QColor action_icon_color READ get_action_icon_color WRITE set_action_icon_color)
        Q_PROPERTY(QColor action_icon_color_hover READ get_action_icon_color_hover WRITE
                       set_action_icon_color_hover)
        Q_PROPERTY(int action_icon_px READ get_action_icon_px WRITE set_action_icon_px)

    public:
        /**
         * @brief Constructs an initially empty log-details inspector.
         * @param parent Parent widget.
         */
        explicit LogDetailsWidget(QWidget* parent = nullptr);

        /** @brief Destroys the generated user interface. */
        ~LogDetailsWidget() override;

        /**
         * @brief Presents one complete log entry.
         * @param entry Entry to inspect.
         */
        auto set_log_entry(const LogEntry& entry) -> void;

        /** @brief Clears the selection and displays the embedded empty state. */
        auto clear_log_entry() -> void;

        /**
         * @brief Reports whether an entry is currently presented.
         * @return True when the inspector contains a selected entry.
         */
        [[nodiscard]] auto has_log_entry() const noexcept -> bool;

        /** @brief Copies all details of the current entry and shows action feedback. */
        auto copy_current_details() -> void;

        /**
         * @brief Returns no preferred extent so the inspector cannot resize its dock.
         * @return An empty preferred size.
         */
        [[nodiscard]] auto sizeHint() const -> QSize override;

        /**
         * @brief Returns no minimum extent so restored dock sizes remain authoritative.
         * @return An empty minimum size.
         */
        [[nodiscard]] auto minimumSizeHint() const -> QSize override;

        /**
         * @brief Returns the normal action icon tint.
         * @return Configured icon color.
         */
        [[nodiscard]] auto get_action_icon_color() const -> QColor;

        /**
         * @brief Sets the normal action icon tint.
         * @param color New icon color.
         */
        auto set_action_icon_color(const QColor& color) -> void;

        /**
         * @brief Returns the hovered action icon tint.
         * @return Configured hover color.
         */
        [[nodiscard]] auto get_action_icon_color_hover() const -> QColor;

        /**
         * @brief Sets the hovered action icon tint.
         * @param color New hover color.
         */
        auto set_action_icon_color_hover(const QColor& color) -> void;

        /**
         * @brief Returns the square action icon size.
         * @return Icon size in pixels.
         */
        [[nodiscard]] auto get_action_icon_px() const noexcept -> int;

        /**
         * @brief Sets the square action icon size.
         * @param size Icon size in pixels.
         */
        auto set_action_icon_px(int size) -> void;

    protected:
        /**
         * @brief Retranslates generated and runtime-created text after language changes.
         * @param event Delivered widget event.
         */
        auto changeEvent(QEvent* event) -> void override;

        /**
         * @brief Adapts the inspector sections to the available dock width.
         * @param event Resize event delivered by Qt.
         */
        auto resizeEvent(QResizeEvent* event) -> void override;

    private:
        /** @brief Creates the compact no-selection presentation inside the UI stack. */
        auto setup_empty_state() -> void;

        /** @brief Connects copy, folder, and field-table actions. */
        auto connect_actions() -> void;

        /** @brief Updates translated text for runtime-configured controls. */
        auto update_action_texts() -> void;

        /** @brief Switches the details splitter between wide and compact arrangements. */
        auto update_details_layout() -> void;

        /** @brief Applies the current entry to every visible details section. */
        auto refresh_entry() -> void;

        /** @brief Rebuilds the additional-field table from the current entry. */
        auto refresh_additional_fields() -> void;

        /** @brief Updates enabled states and tooltips derived from the current entry. */
        auto refresh_actions() -> void;

        /** @brief Rebuilds QSS-tinted copy and external-folder icons. */
        auto update_action_icons() -> void;

        /**
         * @brief Briefly marks a copy button as successfully activated.
         * @param button Copy button receiving visual confirmation.
         */
        auto show_copy_feedback(QToolButton* button) -> void;

        /** @brief Opens a context menu for copying selected additional fields. */
        auto show_fields_context_menu(const QPoint& position) -> void;

        /** @brief Copies the selected additional field rows to the clipboard. */
        auto copy_selected_fields() const -> void;

        /**
         * @brief Copies non-empty text to the system clipboard.
         * @param text Text to copy.
         */
        auto copy_text(const QString& text) const -> void;

        /**
         * @brief Builds a readable complete representation of the selected entry.
         * @return Complete details text, or an empty string without a selection.
         */
        [[nodiscard]] auto build_complete_details_text() const -> QString;

        /**
         * @brief Formats one parsed QVariant for display.
         * @param value Parsed field value.
         * @return Readable value text.
         */
        [[nodiscard]] static auto format_field_value(const QVariant& value) -> QString;

        /**
         * @brief Converts a stable field identifier to a readable fallback label.
         * @param field_id Stable parser field identifier.
         * @return Human-readable label.
         */
        [[nodiscard]] static auto format_field_label(const QString& field_id) -> QString;

        Ui::LogDetailsWidget* ui{nullptr};
        EmptyStateWidget* m_empty_state{nullptr};
        std::optional<LogEntry> m_entry;
        QColor m_action_icon_color{QStringLiteral("#b0bec5")};
        QColor m_action_icon_color_hover{QStringLiteral("#ffffff")};
        int m_action_icon_px{16};
};
