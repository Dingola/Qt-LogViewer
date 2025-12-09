#pragma once

#include <QColor>
#include <QHBoxLayout>
#include <QLabel>
#include <QString>
#include <QToolButton>
#include <QWidget>

/**
 * @file FilesInViewMenuItemWidget.h
 * @brief Declaration of FilesInViewMenuItemWidget.
 */

/**
 * @class FilesInViewMenuItemWidget
 * @brief Compact menu row widget showing a file name with inline actions (Show Only, Hide/Show,
 * Remove).
 *
 * Intended to be embedded into a QMenu via QWidgetAction. The widget renders:
 *  - Left: elided file name label (tooltip shows full path).
 *  - Right: three small icon buttons (Show Only, Hide/Show, Remove).
 *
 * QSS-configurable properties:
 *  - icon_color: base color for icons (default: #666666)
 *      Usage: FilesInViewMenuItemWidget { qproperty-icon_color: #RRGGBB; }
 *  - icon_color_hover: hover/pressed color for icons (default: #42a5f5)
 *      Usage: FilesInViewMenuItemWidget { qproperty-icon_color_hover: #RRGGBB; }
 *  - icon_color_eye: base color for the "eye" icon (overrides icon_color for that button)
 *      Usage: FilesInViewMenuItemWidget { qproperty-icon_color_eye: #RRGGBB; }
 *  - icon_color_eye_off: base color for the "eye-off" icon
 *      Usage: FilesInViewMenuItemWidget { qproperty-icon_color_eye_off: #RRGGBB; }
 *  - icon_color_trash: base color for the "trash" icon
 *      Usage: FilesInViewMenuItemWidget { qproperty-icon_color_trash: #RRGGBB; }
 *  - icon_px: icon size in pixels (square, default: 14)
 *      Usage: FilesInViewMenuItemWidget { qproperty-icon_px: 14; }
 *  - icon_eye_path, icon_eye_off_path, icon_trash_path: SVG resource paths
 *      Usage:
 *        FilesInViewMenuItemWidget {
 *          qproperty-icon_eye_path: ":/Resources/Icons/eye.svg";
 *          qproperty-icon_eye_off_path: ":/Resources/Icons/eye-off.svg";
 *          qproperty-icon_trash_path: ":/Resources/Icons/trash.svg";
 *        }
 *  - label_reserved_px: pixels reserved on the right for buttons/spacers (default: 60)
 *      Usage: FilesInViewMenuItemWidget { qproperty-label_reserved_px: 48; }
 */
class FilesInViewMenuItemWidget: public QWidget
{
        Q_OBJECT
        Q_PROPERTY(QColor icon_color READ get_icon_color WRITE set_icon_color)
        Q_PROPERTY(QColor icon_color_hover READ get_icon_color_hover WRITE set_icon_color_hover)
        Q_PROPERTY(QColor icon_color_eye READ get_icon_color_eye WRITE set_icon_color_eye)
        Q_PROPERTY(
            QColor icon_color_eye_off READ get_icon_color_eye_off WRITE set_icon_color_eye_off)
        Q_PROPERTY(QColor icon_color_trash READ get_icon_color_trash WRITE set_icon_color_trash)
        Q_PROPERTY(int icon_px READ get_icon_px WRITE set_icon_px)
        Q_PROPERTY(QString icon_eye_path READ get_icon_eye_path WRITE set_icon_eye_path)
        Q_PROPERTY(QString icon_eye_off_path READ get_icon_eye_off_path WRITE set_icon_eye_off_path)
        Q_PROPERTY(QString icon_trash_path READ get_icon_trash_path WRITE set_icon_trash_path)
        Q_PROPERTY(int label_reserved_px READ get_label_reserved_px WRITE set_label_reserved_px)

    public:
        /**
         * @brief Constructs the menu item widget.
         * @param parent Optional parent.
         */
        explicit FilesInViewMenuItemWidget(QWidget* parent = nullptr);

        /**
         * @brief Destroys the widget.
         */
        ~FilesInViewMenuItemWidget() override = default;

        /**
         * @brief Sets the file path represented by this menu item and updates the label.
         * @param file_path Absolute or relative file path.
         */
        auto set_file_path(const QString& file_path) -> void;

        /**
         * @brief Returns the file path represented by this menu item.
         * @return Full file path string.
         */
        [[nodiscard]] auto get_file_path() const -> QString;

        /**
         * @brief Updates the visibility toggle presentation and tooltip.
         *
         * When hidden is true, the tooltip becomes "Show this file".
         * When false, the tooltip is "Hide this file".
         * The button remains enabled in both states.
         *
         * @param hidden True if file is currently hidden by filters.
         */
        auto set_hidden_state(bool hidden) -> void;

        /**
         * @brief Sets icon resource paths for the inline action buttons and reapplies icons.
         * @param eye Path to "Show Only" icon (eye).
         * @param eye_off Path to "Hide" icon (eye-off).
         * @param trash Path to "Remove" icon (trash).
         */
        auto set_icon_paths(const QString& eye, const QString& eye_off,
                            const QString& trash) -> void;

        /**
         * @brief Sets the base icon color for inline action buttons and reapplies icons.
         * @param color Base color.
         */
        auto set_icon_color(const QColor& color) -> void;

        /**
         * @brief Gets the base icon color for inline action buttons.
         * @return Current base color.
         */
        [[nodiscard]] auto get_icon_color() const -> QColor;

        /**
         * @brief Sets the hover/pressed icon color and reapplies icons when hovering.
         * @param color Hover color.
         */
        auto set_icon_color_hover(const QColor& color) -> void;

        /**
         * @brief Gets the hover/pressed icon color.
         * @return Current hover color.
         */
        [[nodiscard]] auto get_icon_color_hover() const -> QColor;

        /**
         * @brief Sets the base color for the "eye" icon and reapplies icons.
         * @param color Eye icon base color.
         */
        auto set_icon_color_eye(const QColor& color) -> void;

        /**
         * @brief Gets the base color for the "eye" icon.
         * @return Current eye icon base color.
         */
        [[nodiscard]] auto get_icon_color_eye() const -> QColor;

        /**
         * @brief Sets the base color for the "eye-off" icon and reapplies icons.
         * @param color Eye-off icon base color.
         */
        auto set_icon_color_eye_off(const QColor& color) -> void;

        /**
         * @brief Gets the base color for the "eye-off" icon.
         * @return Current eye-off icon base color.
         */
        [[nodiscard]] auto get_icon_color_eye_off() const -> QColor;

        /**
         * @brief Sets the base color for the "trash" icon and reapplies icons.
         * @param color Trash icon base color.
         */
        auto set_icon_color_trash(const QColor& color) -> void;

        /**
         * @brief Gets the base color for the "trash" icon.
         * @return Current trash icon base color.
         */
        [[nodiscard]] auto get_icon_color_trash() const -> QColor;

        /**
         * @brief Sets the icon size (square px) and reapplies icons.
         * @param px Icon size in pixels. Values < 1 are clamped to 1.
         */
        auto set_icon_px(int px) -> void;

        /**
         * @brief Gets the icon size (square px).
         * @return Icon size in pixels.
         */
        [[nodiscard]] auto get_icon_px() const -> int;

        /**
         * @brief Sets the path to the "Show Only" icon and reapplies icons.
         * @param path Resource path to eye SVG.
         */
        auto set_icon_eye_path(const QString& path) -> void;

        /**
         * @brief Gets the path to the "Show Only" icon.
         * @return Resource path to eye SVG.
         */
        [[nodiscard]] auto get_icon_eye_path() const -> QString;

        /**
         * @brief Sets the path to the "Hide" icon and reapplies icons.
         * @param path Resource path to eye-off SVG.
         */
        auto set_icon_eye_off_path(const QString& path) -> void;

        /**
         * @brief Gets the path to the "Hide" icon.
         * @return Resource path to eye-off SVG.
         */
        [[nodiscard]] auto get_icon_eye_off_path() const -> QString;

        /**
         * @brief Sets the path to the "Remove" icon and reapplies icons.
         * @param path Resource path to trash SVG.
         */
        auto set_icon_trash_path(const QString& path) -> void;

        /**
         * @brief Gets the path to the "Remove" icon.
         * @return Resource path to trash SVG.
         */
        [[nodiscard]] auto get_icon_trash_path() const -> QString;

        /**
         * @brief Sets the path to the "Show Only" action icon and reapplies icons.
         *        This icon is used for the "Show only this file" button, distinct from visibility
         * toggle.
         * @param path Resource path to the show-only SVG.
         */
        auto set_icon_show_only_path(const QString& path) -> void;

        /**
         * @brief Gets the path to the "Show Only" action icon.
         * @return Resource path to the show-only SVG.
         */
        [[nodiscard]] auto get_icon_show_only_path() const -> QString;

        /**
         * @brief Returns the pixels reserved on the right side (buttons/spacers).
         * @return Reserved pixel count.
         */
        [[nodiscard]] auto get_label_reserved_px() const -> int;

        /**
         * @brief Sets the pixels reserved on the right side (buttons/spacers).
         * @param px Reserved pixel count (clamped to >= 0).
         */
        auto set_label_reserved_px(int px) -> void;

    signals:
        /**
         * @brief Emitted when "Show Only" action is requested for the file.
         * @param file_path Target file path.
         */
        void show_only_requested(const QString& file_path);

        /**
         * @brief Emitted when the visibility toggle (Hide/Show) is requested for the file.
         * @param file_path Target file path.
         */
        void toggle_visibility_requested(const QString& file_path);

        /**
         * @brief Emitted when "Remove" action is requested for the file.
         * @param file_path Target file path.
         */
        void remove_requested(const QString& file_path);

    protected:
        /**
         * @brief Handles hover/press events on inline buttons to swap icon color.
         * @param watched Watched object (buttons).
         * @param event The event to handle.
         * @return Always false; the event is not consumed.
         */
        bool eventFilter(QObject* watched, QEvent* event) override;

        /**
         * @brief Re-elides label when the row width changes.
         * @param event Resize event.
         */
        void resizeEvent(QResizeEvent* event) override;

    private:
        /**
         * @brief Builds layout and wiring for controls.
         */
        auto setup_ui() -> void;

        /**
         * @brief Updates the label with an elided file name and tooltip with full path.
         */
        auto update_label_elided() -> void;

        /**
         * @brief Applies icons to inline buttons with the current color/size/paths.
         */
        auto apply_icons() -> void;

        /**
         * @brief Updates a single button icon based on hover state.
         * @param btn Target button.
         * @param path SVG resource path.
         * @param hovered True to use hover color; false for base color.
         */
        auto update_button_icon(QToolButton* btn, const QString& path, bool hovered) -> void;

        /**
         * @brief Builds a colored QIcon for an SVG at the requested color/size.
         * @param svg_path SVG resource path.
         * @param color Target color.
         * @return Colored QIcon.
         */
        [[nodiscard]] auto make_icon(const QString& svg_path, const QColor& color) const -> QIcon;

    private:
        QString m_file_path;

        // Icon configuration (QSS-settable via Q_PROPERTY)
        QColor m_icon_color = QColor("#666666");
        QColor m_icon_color_hover = QColor("#42a5f5");
        QColor m_icon_color_eye = QColor();      ///< Optional override for eye icon
        QColor m_icon_color_eye_off = QColor();  ///< Optional override for eye-off icon
        QColor m_icon_color_trash = QColor();    ///< Optional override for trash icon
        int m_icon_px = 14;

        QString m_icon_eye = QStringLiteral(":/Resources/Icons/eye.svg");
        QString m_icon_eye_off = QStringLiteral(":/Resources/Icons/eye-off.svg");
        QString m_icon_trash = QStringLiteral(":/Resources/Icons/trash.svg");
        QString m_icon_show_only = QStringLiteral(":/Resources/Icons/show-only.svg");

        int m_label_reserved_px = 60;
        bool m_is_hidden_effective = false;

        QLabel* m_label = nullptr;
        QToolButton* m_btn_show_only = nullptr;
        QToolButton* m_btn_toggle_visibility = nullptr;
        QToolButton* m_btn_remove = nullptr;
        QHBoxLayout* m_layout = nullptr;
};
