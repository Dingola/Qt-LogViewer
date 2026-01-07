#pragma once

#include <QColor>
#include <QMargins>
#include <QString>
#include <QStringList>
#include <QToolButton>
#include <QWidget>

namespace Ui
{
class SearchBarWidget;
}

/**
 * @class SearchBarWidget
 * @brief Widget for searching log entries by text, field, and regex.
 *
 * Provides controls for entering search text, selecting the search field, and enabling regex.
 */
class SearchBarWidget: public QWidget
{
        Q_OBJECT
        /**
         * @brief Normal (non-hover) color for the clear icon.
         *
         * Can be set via QSS:
         *   SearchBarWidget { qproperty-clear_icon_color: #RRGGBB; }
         */
        Q_PROPERTY(QColor clear_icon_color READ get_clear_icon_color WRITE set_clear_icon_color)
        /**
         * @brief Hover/pressed color for the clear icon.
         *
         * Can be set via QSS:
         *   SearchBarWidget { qproperty-clear_icon_color_hover: #RRGGBB; }
         */
        Q_PROPERTY(QColor clear_icon_color_hover READ get_clear_icon_color_hover WRITE
                       set_clear_icon_color_hover)
        /**
         * @brief Square icon size (px) for the clear icon.
         *
         * Can be set via QSS:
         *   SearchBarWidget { qproperty-clear_icon_px: 16; }
         */
        Q_PROPERTY(int clear_icon_px READ get_clear_icon_px WRITE set_clear_icon_px)
        /**
         * @brief Padding (px) between the end of the typed text and the clear button.
         *
         * Can be set via QSS:
         *   SearchBarWidget { qproperty-clear_button_padding: 8; }
         */
        Q_PROPERTY(
            int clear_button_padding READ get_clear_button_padding WRITE set_clear_button_padding)

    public:
        /**
         * @brief Constructs a SearchBarWidget object.
         *
         * Initializes the search bar widget and its UI.
         *
         * @param parent The parent widget, or nullptr if this is a top-level widget.
         */
        explicit SearchBarWidget(QWidget* parent = nullptr);

        /**
         * @brief Destroys the SearchBarWidget object.
         *
         * Cleans up any resources used by the search bar widget.
         */
        ~SearchBarWidget() override;

        /**
         * @brief Sets the available search fields in the combo box.
         *
         * @param fields The list of search fields.
         */
        auto set_search_fields(const QStringList& fields) -> void;

        /**
         * @brief Sets the placeholder text for the search line edit.
         *
         * @param text The placeholder text.
         */
        auto set_search_placeholder(const QString& text) -> void;

        /**
         * @brief Returns the current search text.
         *
         * @return The search text.
         */
        [[nodiscard]] auto get_search_text() const -> QString;

        /**
         * @brief Returns the currently selected search field.
         *
         * @return The search field.
         */
        [[nodiscard]] auto get_search_field() const -> QString;

        /**
         * @brief Returns whether regex is enabled.
         *
         * @return True if regex is enabled, false otherwise.
         */
        [[nodiscard]] auto get_use_regex() const -> bool;

        /**
         * @brief Returns whether live search is enabled.
         *
         * When enabled, a search is requested automatically while typing without
         * pressing Enter or the Search button.
         *
         * @return True if live search is enabled, false otherwise.
         */
        [[nodiscard]] auto get_use_live_search() const -> bool;

        /**
         * @brief Gets the normal (non-hover) clear icon color.
         * @return The current color.
         */
        [[nodiscard]] auto get_clear_icon_color() const -> QColor;

        /**
         * @brief Sets the normal (non-hover) clear icon color and refreshes the icon.
         * @param color The new color to use.
         */
        auto set_clear_icon_color(const QColor& color) -> void;

        /**
         * @brief Gets the hover/pressed clear icon color.
         * @return The current hover color.
         */
        [[nodiscard]] auto get_clear_icon_color_hover() const -> QColor;

        /**
         * @brief Sets the hover/pressed clear icon color.
         * @param color The new hover color to use.
         */
        auto set_clear_icon_color_hover(const QColor& color) -> void;

        /**
         * @brief Gets the clear icon square size in pixels.
         * @return The icon size.
         */
        [[nodiscard]] auto get_clear_icon_px() const -> int;

        /**
         * @brief Sets the clear icon size in pixels and updates layout.
         * @param px The new size (values < 1 are clamped to 1).
         */
        auto set_clear_icon_px(int px) -> void;

        /**
         * @brief Gets the padding between text end and clear button.
         * @return The padding in pixels.
         */
        [[nodiscard]] auto get_clear_button_padding() const -> int;

        /**
         * @brief Sets the padding between text end and clear button and updates layout.
         * @param px The new padding in pixels (values < 0 are clamped to 0).
         */
        auto set_clear_button_padding(int px) -> void;

    signals:
        /**
         * @brief Emitted when a search is requested.
         * @param text The search text.
         * @param field The search field.
         * @param regex True if regex is enabled.
         */
        void search_requested(const QString& text, const QString& field, bool regex);

        /**
         * @brief Emitted when the search text changes.
         * @param text The new search text.
         */
        void search_text_changed(const QString& text);

        /**
         * @brief Emitted when the search field changes.
         * @param field The new search field.
         */
        void search_field_changed(const QString& field);

        /**
         * @brief Emitted when the regex checkbox is toggled.
         * @param enabled True if regex is enabled.
         */
        void regex_toggled(bool enabled);

        /**
         * @brief Emitted when the live search checkbox is toggled.
         * @param enabled True if live search is enabled.
         */
        void live_search_toggled(bool enabled);

    protected:
        /**
         * @brief Handles change events to update the UI.
         * @param event The change event.
         */
        auto changeEvent(QEvent* event) -> void override;

        /**
         * @brief Ensures clear button is repositioned when the widget resizes.
         * @param event The resize event.
         */
        auto resizeEvent(QResizeEvent* event) -> void override;

        /**
         * @brief Handles hover/press events for the clear button to apply hover color.
         * @param watched The watched object (expected: clear button).
         * @param event The event to handle.
         * @return true to consume; false to propagate (always false here).
         */
        auto eventFilter(QObject* watched, QEvent* event) -> bool override;

    private:
        /**
         * @brief Creates and configures the clear button as a child of the line edit.
         */
        auto init_clear_button() -> void;

        /**
         * @brief Shows/hides and repositions the clear button based on current text.
         * @param text The current line edit text.
         */
        auto update_clear_button_visibility(const QString& text) -> void;

        /**
         * @brief Ensures the line edit has a right margin large enough for the clear button.
         */
        auto ensure_text_right_margin() -> void;

        /**
         * @brief Positions the clear button next to the visible text end (LTR).
         */
        auto position_clear_button() -> void;

        /**
         * @brief Updates the clear button icon using the current color and size.
         */
        auto update_clear_button_icon() -> void;

    private:
        Ui::SearchBarWidget* ui;

        // Clear button shown inside the line edit, positioned next to typed text.
        QToolButton* m_clear_button = nullptr;

        // QSS-controllable properties (defaults chosen to match project style).
        QColor m_clear_icon_color = QColor("#666666");
        QColor m_clear_icon_color_hover = QColor("#42a5f5");
        int m_clear_icon_px = 16;
        int m_clear_button_padding = 8;

        // Stored original text margins so they can be restored when the clear button is hidden.
        QMargins m_original_text_margins;
};
