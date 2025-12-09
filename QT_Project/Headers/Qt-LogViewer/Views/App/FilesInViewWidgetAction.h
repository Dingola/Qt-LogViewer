#pragma once

#include <QColor>
#include <QWidgetAction>

/**
 * @file FilesInViewWidgetAction.h
 * @brief Declaration of FilesInViewWidgetAction.
 */

/**
 * @class FilesInViewWidgetAction
 * @brief QWidgetAction hosting a FilesInViewMenuItemWidget with configurable icon
 * paths/colors/size.
 */
class FilesInViewWidgetAction: public QWidgetAction
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs the action.
         * @param parent Optional parent object.
         */
        explicit FilesInViewWidgetAction(QObject* parent = nullptr);

        /**
         * @brief Destroys the action.
         */
        ~FilesInViewWidgetAction() override = default;

        /**
         * @brief Sets the file path carried by this action's embedded widget.
         * @param file_path File path to forward to the embedded widget.
         */
        auto set_file_path(const QString& file_path) -> void;

        /**
         * @brief Returns the file path carried by this action's embedded widget.
         * @return File path string.
         */
        [[nodiscard]] auto get_file_path() const -> QString;

        /**
         * @brief Sets icon resource paths (optional; defaults are provided).
         * @param eye Path to "Show Only" icon.
         * @param eye_off Path to "Hide" icon.
         * @param trash Path to "Remove" icon.
         */
        auto set_icon_paths(const QString& eye, const QString& eye_off,
                            const QString& trash) -> void;

        /**
         * @brief Sets the base icon color for inline buttons.
         * @param color Base color.
         */
        auto set_icon_color(const QColor& color) -> void;

        /**
         * @brief Gets the base icon color for inline buttons.
         * @return Current base color.
         */
        [[nodiscard]] auto get_icon_color() const -> QColor;

        /**
         * @brief Sets the hover/pressed icon color for inline buttons.
         * @param color Hover color.
         */
        auto set_icon_color_hover(const QColor& color) -> void;

        /**
         * @brief Gets the hover/pressed icon color for inline buttons.
         * @return Current hover color.
         */
        [[nodiscard]] auto get_icon_color_hover() const -> QColor;

        /**
         * @brief Sets the icon size in pixels (square).
         * @param px Icon size in pixels. Values < 1 are clamped to 1.
         */
        auto set_icon_px(int px) -> void;

        /**
         * @brief Gets the icon size in pixels (square).
         * @return Icon size in pixels.
         */
        [[nodiscard]] auto get_icon_px() const -> int;

        /**
         * @brief Sets whether the file represented by this action is effectively hidden.
         *
         * Effective hidden means either explicitly hidden by filters, or hidden due to an
         * active "show only" filter targeting a different file. This flag is forwarded to
         * the embedded row widget to toggle the Hide/Show presentation (tooltip and icon).
         *
         * @param hidden True if effectively hidden; false otherwise.
         */
        auto set_hidden_effective(bool hidden) -> void;

        /**
         * @brief Returns whether the file represented by this action is effectively hidden.
         * @return True if effectively hidden; false otherwise.
         */
        [[nodiscard]] auto get_hidden_effective() const -> bool;

    signals:
        /**
         * @brief Forwarded from embedded widget when actions are requested.
         */
        void show_only_requested(const QString& file_path);
        /**
         * @brief Forwarded from embedded widget when actions are requested.
         */
        void toggle_visibility_requested(const QString& file_path);
        /**
         * @brief Forwarded from embedded widget when actions are requested.
         */
        void remove_requested(const QString& file_path);

    protected:
        /**
         * @brief Creates the embedded widget instance and wires its signals to this action.
         * @param parent Menu or parent widget.
         * @return Newly created widget hosted by the QWidgetAction.
         */
        QWidget* createWidget(QWidget* parent) override;

        /**
         * @brief Deletes the embedded widget safely using deleteLater().
         * @param widget Widget instance to delete.
         */
        auto deleteWidget(QWidget* widget) -> void override;

    private:
        QString m_file_path;

        // Resource path overrides (optional)
        QString m_icon_eye;
        QString m_icon_eye_off;
        QString m_icon_trash;

        // Coloring and sizing (optional overrides; defaults match QSS recommendation)
        QColor m_icon_color = QColor("#666666");
        QColor m_icon_color_hover = QColor("#42a5f5");
        int m_icon_px = 14;

        /**
         * @brief Effective hidden flag propagated to the embedded widget.
         */
        bool m_hidden_effective = false;
};
