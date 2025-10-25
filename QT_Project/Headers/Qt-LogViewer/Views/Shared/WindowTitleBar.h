#pragma once

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
};
