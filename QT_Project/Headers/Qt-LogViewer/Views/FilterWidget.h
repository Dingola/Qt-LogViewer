#pragma once

#include <QSet>
#include <QString>
#include <QWidget>

namespace Ui
{
class FilterWidget;
}

/**
 * @class FilterWidget
 * @brief Widget for filtering log entries by application and log level.
 *
 * Provides controls for selecting the application and log levels to filter the log view.
 */
class FilterWidget: public QWidget
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs a FilterWidget object.
         *
         * Initializes the filter widget and its UI.
         *
         * @param parent The parent widget, or nullptr if this is a top-level widget.
         */
        explicit FilterWidget(QWidget* parent = nullptr);

        /**
         * @brief Destroys the FilterWidget object.
         *
         * Cleans up any resources used by the filter widget.
         */
        ~FilterWidget() override;

        /**
         * @brief Populates the application filter combo box.
         *
         * Clears the combo box, adds a default entry, and populates it with the provided
         * application names. Disables the combo box if no application names are provided.
         *
         * @param app_names A set of application names to populate the combo box.
         * @param default_text The default entry text.
         * @param default_tooltip The tooltip for the default entry.
         */
        auto set_app_names(const QSet<QString>& app_names, const QString& default_text,
                           const QString& default_tooltip) -> void;

        /**
         * @brief Sets the application names in the combo box.
         *
         * This overload sets the application names without a default entry.
         *
         * @param app_names The set of application names to populate the combo box.
         */
        auto set_app_names(const QSet<QString>& app_names) -> void;

        /**
         * @brief Sets the checked state of log level checkboxes.
         *
         * @param levels The set of log levels to check.
         */
        auto set_log_levels(const QSet<QString>& levels) -> void;

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

    protected:
        /**
         * @brief Handles change events to update the UI.
         * @param event The change event.
         */
        auto changeEvent(QEvent* event) -> void override;

    private:
        Ui::FilterWidget* ui;
};
