#pragma once

#include <QString>
#include <QWidget>

namespace Ui
{
class LogLevelFilterItemWidget;
}

/**
 * @class LogLevelFilterItemWidget
 * @brief Widget representing a single log level filter item (checkbox + count label).
 *
 * Provides a checkbox for enabling/disabling a log level filter and a label for displaying the
 * count.
 */
class LogLevelFilterItemWidget: public QWidget
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs a LogLevelFilterItemWidget object.
         *
         * Initializes the log level filter item widget and its UI.
         *
         * @param parent The parent widget, or nullptr if this is a top-level widget.
         */
        explicit LogLevelFilterItemWidget(QWidget* parent = nullptr);

        /**
         * @brief Destroys the LogLevelFilterItemWidget object.
         *
         * Cleans up any resources used by the widget.
         */
        ~LogLevelFilterItemWidget() override;

        /**
         * @brief Sets the log level name and checkbox text.
         *        Also sets the checkbox objectName for QSS styling.
         * @param level_name The log level name (e.g., "Trace", "Debug").
         */
        auto set_log_level_name(const QString& level_name) -> void;

        /**
         * @brief Sets the count label for the log level.
         * @param count_text The text to display (e.g., "42", "1.2K").
         */
        auto set_count_text(const QString& count_text) -> void;

        /**
         * @brief Sets the count label for the log level.
         * @param count The integer count to display (e.g., 42, 1200).
         */
        auto set_count(int count) -> void;

        /**
         * @brief Sets the checked state of the checkbox.
         * @param checked True to check, false to uncheck.
         */
        auto set_checked(bool checked) -> void;

        /**
         * @brief Returns whether the checkbox is checked.
         * @return True if checked, false otherwise.
         */
        [[nodiscard]] auto get_checked() const -> bool;

        /**
         * @brief Returns the log level name.
         * @return The log level name.
         */
        [[nodiscard]] auto get_log_level_name() const -> QString;

        /**
         * @brief Resets the count label to "0" and unchecks the checkbox.
         */
        auto clear() -> void;

    signals:
        /**
         * @brief Emitted when the checkbox is toggled.
         * @param checked The new checked state.
         */
        void toggled(bool checked);

    protected:
        /**
         * @brief Handles change events to update the UI.
         * @param event The change event.
         */
        auto changeEvent(QEvent* event) -> void override;

    private:
        Ui::LogLevelFilterItemWidget* ui;
        QString m_log_level_name;
};
