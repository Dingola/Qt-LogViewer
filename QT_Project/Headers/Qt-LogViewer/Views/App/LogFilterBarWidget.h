#pragma once

#include <QSet>
#include <QString>
#include <QStringList>
#include <QWidget>

namespace Ui
{
class LogFilterBarWidget;
}

class LogFilterWidget;
class SearchBarWidget;

/**
 * @class LogFilterBarWidget
 * @brief Composite widget for log filtering and searching.
 *
 * Combines LogFilterWidget and SearchBarWidget to provide a unified filter/search bar.
 */
class LogFilterBarWidget: public QWidget
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs a LogFilterBarWidget object.
         *
         * Initializes the filter bar widget and its UI.
         *
         * @param parent The parent widget, or nullptr if this is a top-level widget.
         */
        explicit LogFilterBarWidget(QWidget* parent = nullptr);

        /**
         * @brief Destroys the LogFilterBarWidget object.
         *
         * Cleans up any resources used by the filter bar widget.
         */
        ~LogFilterBarWidget() override;

        /**
         * @brief Sets the application names in the combo box.
         *
         * This overload sets the application names without a default entry.
         *
         * @param app_names The set of application names to populate the combo box.
         */
        auto set_app_names(const QSet<QString>& app_names) -> void;

        /**
         * @brief Sets the currently selected application name in the combo box.
         *
         * If the provided application name does not exist in the combo box, no selection is made.
         *
         * @param app_name The application name to select.
         */
        auto set_current_app_name_filter(const QString& app_name) -> void;

        /**
         * @brief Sets the checked state of log level checkboxes in the filter widget.
         *
         * @param levels The set of log levels to check.
         */
        auto set_log_levels(const QSet<QString>& levels) -> void;

        /**
         * @brief Sets the available search fields in the search bar widget.
         *
         * @param fields The list of search fields.
         */
        auto set_search_fields(const QStringList& fields) -> void;

        /**
         * @brief Sets the placeholder text for the search line edit in the search bar widget.
         *
         * @param text The placeholder text.
         */
        auto set_search_placeholder(const QString& text) -> void;

        /**
         * @brief Returns the currently selected application name.
         *
         * @return The selected application name.
         */
        [[nodiscard]] auto get_current_app_name() const -> QString;

        /**
         * @brief Returns the currently selected log levels.
         *
         * @return The set of selected log levels.
         */
        [[nodiscard]] auto get_current_log_levels() const -> QSet<QString>;

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
         * @brief Returns a pointer to the contained LogFilterWidget.
         *
         * @return Pointer to LogFilterWidget.
         */
        [[nodiscard]] auto get_filter_widget() const -> LogFilterWidget*;

        /**
         * @brief Returns a pointer to the contained SearchBarWidget.
         *
         * @return Pointer to SearchBarWidget.
         */
        [[nodiscard]] auto get_search_bar_widget() const -> SearchBarWidget*;

    signals:
        /**
         * @brief Emitted when the application filter selection changes.
         * @param app_name The selected application name.
         */
        void app_filter_changed(const QString& app_name);

        /**
         * @brief Emitted when the log level filter selection changes.
         * @param levels The set of selected log levels.
         */
        void log_level_filter_changed(const QSet<QString>& levels);

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

    protected:
        /**
         * @brief Handles change events to update the UI.
         * @param event The change event.
         */
        auto changeEvent(QEvent* event) -> void override;

    private:
        Ui::LogFilterBarWidget* ui;
};
