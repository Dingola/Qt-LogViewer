#pragma once

#include <QColor>
#include <QIcon>
#include <QMenu>
#include <QWidget>

class QLabel;
class QPushButton;
class QMenuBar;
class QHBoxLayout;
class QVBoxLayout;

/**
 * @class WindowTitleBar
 * @brief Custom window title bar widget with minimize, maximize/restore, close, and menu support.
 */
class WindowTitleBar: public QWidget
{
        Q_OBJECT
        /**
         * @brief Color used to render SVG icons for the window control buttons.
         *
         * This property can be set via QSS using:
         *   QWidget#windowtitlebar { qproperty-window_button_color: #42a5f5; }
         */
        Q_PROPERTY(
            QColor window_button_color READ get_window_button_color WRITE set_window_button_color)
        /**
         * @brief Size (in pixels) used for the window control button icons (square).
         *
         * Default is 18. Can be set via QSS:
         *   QWidget#windowtitlebar { qproperty-window_button_icon_px: 20; }
         */
        Q_PROPERTY(int window_button_icon_px READ get_window_button_icon_px WRITE
                       set_window_button_icon_px)

    public:
        /**
         * @brief Row location for placing optional widgets (menubar/custom widget).
         */
        enum RowPosition
        {
            Top,    ///< Place on the top row between title and window buttons.
            Bottom  ///< Place on a second row below the icon+title.
        };

        /**
         * @brief Constructs a WindowTitleBar object.
         * @param parent The parent widget, or nullptr if this is a top-level widget.
         */
        explicit WindowTitleBar(QWidget* parent = nullptr);

        /**
         * @brief Sets the small icon displayed at the far left of the title bar.
         * @param icon The icon to display.
         */
        auto set_icon(const QIcon& icon) -> void;

        /**
         * @brief Sets the window title text.
         * @param title The title to display.
         */
        auto set_title(const QString& title) -> void;

        /**
         * @brief Set a menubar to be embedded in the title bar and re-parent it to this widget.
         *
         * The menubar is placed either on the top row (between the title and the window buttons)
         * or on a second row under the icon+title depending on set_menubar_row().
         *
         * The method is virtual so derived classes can override the adoption behavior.
         *
         * @param menubar Pointer to a QMenuBar to embed (may be nullptr to remove).
         */
        virtual auto set_menubar(QMenuBar* menubar) -> void;

        /**
         * @brief Choose the row where the menubar should be placed.
         * @param row Top to place between title and buttons; Bottom to place under icon+title.
         */
        auto set_menubar_row(RowPosition row) -> void;

        /**
         * @brief Set a custom widget to appear in the title bar and re-parent it to this widget.
         *
         * The custom widget is placed either:
         *  - on the top row, after the menubar (if present) and before the window buttons, or
         *  - on the second row, to the right of the menubar (if present).
         *
         * @param widget Pointer to a QWidget to embed (may be nullptr to remove).
         */
        auto set_custom_widget(QWidget* widget) -> void;

        /**
         * @brief Choose the row where the custom widget should be placed.
         * @param row Top to place between title/menu and buttons; Bottom to place under icon+title.
         */
        auto set_custom_widget_row(RowPosition row) -> void;

        /**
         * @brief Returns the embedded menubar if set.
         * @return QMenuBar* Pointer to the menubar or nullptr.
         */
        [[nodiscard]] auto get_menubar() const -> QMenuBar*;

        /**
         * @brief Returns the custom widget if set.
         * @return QWidget* Pointer to the custom widget or nullptr.
         */
        [[nodiscard]] auto get_custom_widget() const -> QWidget*;

        /**
         * @brief Returns the minimize button.
         * @return Pointer to the minimize QPushButton.
         */
        [[nodiscard]] auto get_minimize_button() const -> QPushButton*;

        /**
         * @brief Returns the maximize button.
         * @return Pointer to the maximize/restore QPushButton.
         */
        [[nodiscard]] auto get_maximize_button() const -> QPushButton*;

        /**
         * @brief Returns the close button.
         * @return Pointer to the close QPushButton.
         */
        [[nodiscard]] auto get_close_button() const -> QPushButton*;

        /**
         * @brief Returns the current color used to render window button SVG icons.
         * @return The current window button color.
         */
        [[nodiscard]] auto get_window_button_color() const -> QColor;

        /**
         * @brief Sets the color used to render window button SVG icons.
         *
         * Setting this property will re-render and update all window button icons.
         * Can be set via QSS as: qproperty-window_button_color: #RRGGBB;
         *
         * @param color The new color to apply to the icons.
         */
        auto set_window_button_color(const QColor& color) -> void;

        /**
         * @brief Returns the current icon size in pixels for window buttons.
         * @return The icon size in pixels.
         */
        [[nodiscard]] auto get_window_button_icon_px() const -> int;

        /**
         * @brief Sets the icon size in pixels for window buttons (square).
         *
         * Setting this property updates button icon sizes and re-renders SVG icons.
         * Can be set via QSS as: qproperty-window_button_icon_px: 18;
         *
         * @param px The new icon size in pixels. Values < 1 are clamped to 1.
         */
        auto set_window_button_icon_px(int px) -> void;

    protected:
        /**
         * @brief Handles mouse double-click events for maximize/restore.
         * @param event The mouse event.
         */
        void mouseDoubleClickEvent(QMouseEvent* event) override;

        /**
         * @brief Handles mouse press events for drag start.
         * @param event The mouse event.
         */
        void mousePressEvent(QMouseEvent* event) override;

        /**
         * @brief Install an event filter on the top-level window when shown.
         * @param event The show event.
         */
        void showEvent(QShowEvent* event) override;

        /**
         * @brief Intercepts top-level window state changes to update the max/restore icon.
         * @param watched The object being watched (expected: top-level window).
         * @param event The event being filtered.
         * @return true if handled; false otherwise.
         */
        bool eventFilter(QObject* watched, QEvent* event) override;

    private:
        /**
         * @brief Rebuild the layout to reflect current menubar/custom widget placement.
         *
         * Rules:
         *  - Top row order is [Icon][Title][MenuBar?][Custom?][...][Min][Max][Close].
         *  - Bottom row order is [MenuBar?][Custom?].
         *  - When only one of them is set to Bottom, it appears alone on the second row.
         */
        auto rebuild_layout() -> void;

        /**
         * @brief Apply the current icon size to all window buttons.
         */
        auto apply_button_icon_size() -> void;

        /**
         * @brief Re-color and set all window button icons according to the current property.
         */
        auto update_button_icons() -> void;

        /**
         * @brief Update the maximize/restore button icon based on the actual window state.
         *
         * When the window is maximized, the restore icon is shown; otherwise the maximize icon.
         */
        auto update_maximize_restore_icon() -> void;

    signals:
        /**
         * @brief Emitted when the minimize button is clicked.
         */
        void minimize_requested();

        /**
         * @brief Emitted when the maximize button is clicked.
         */
        void maximize_requested();

        /**
         * @brief Emitted when the restore button is clicked.
         */
        void restore_requested();

        /**
         * @brief Emitted when the close button is clicked.
         */
        void close_requested();

        /**
         * @brief Emitted when the user starts dragging the title bar.
         * @param global_pos The global mouse position at drag start.
         */
        void drag_started(const QPoint& global_pos);

    private:
        QLabel* m_icon_label = nullptr;  ///< Left-aligned app icon
        QLabel* m_title_label = nullptr;
        QPushButton* m_minimize_button = nullptr;
        QPushButton* m_maximize_button = nullptr;
        QPushButton* m_close_button = nullptr;

        QMenuBar* m_menu_bar = nullptr;
        QWidget* m_custom_widget = nullptr;

        QVBoxLayout* m_root_layout = nullptr;
        QHBoxLayout* m_top_row = nullptr;
        QHBoxLayout* m_bottom_row = nullptr;

        RowPosition m_menubar_row = RowPosition::Top;
        RowPosition m_custom_row = RowPosition::Top;

        QColor m_window_button_color = QColor("#000000");
        int m_window_button_icon_px = 18;  ///< Default icon size in pixels
};
