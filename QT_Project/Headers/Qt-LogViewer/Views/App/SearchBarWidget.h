#pragma once

#include <QString>
#include <QStringList>
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

    private:
        Ui::SearchBarWidget* ui;
};
