#include "Qt-LogViewer/Views/Shared/WindowTitleBar.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenuBar>
#include <QMouseEvent>
#include <QPushButton>
#include <QShowEvent>
#include <QSize>
#include <QVBoxLayout>

#include "Qt-LogViewer/Services/UiUtils.h"

/**
 * @brief Constructs a WindowTitleBar object.
 * @param parent The parent widget, or nullptr if this is a top-level widget.
 */
WindowTitleBar::WindowTitleBar(QWidget* parent): QWidget(parent)
{
    setObjectName("windowtitlebar");
    setMinimumHeight(36);

    m_icon_label = new QLabel(this);
    m_icon_label->setObjectName("windowtitlebar_icon");
    m_icon_label->setFixedSize(20, 20);
    m_icon_label->setScaledContents(true);

    m_title_label = new QLabel(tr("AppWindow"), this);
    m_title_label->setObjectName("windowtitlebar_title");
    m_title_label->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

    m_minimize_button = new QPushButton(this);
    m_minimize_button->setObjectName("windowtitlebar_minimize_button");
    m_maximize_button = new QPushButton(this);
    m_maximize_button->setObjectName("windowtitlebar_maximize_button");
    m_close_button = new QPushButton(this);
    m_close_button->setObjectName("windowtitlebar_close_button");

    for (auto btn: {m_minimize_button, m_maximize_button, m_close_button})
    {
        btn->setFixedSize(28, 28);
        btn->setFlat(true);
    }
    apply_button_icon_size();

    m_root_layout = new QVBoxLayout(this);
    m_root_layout->setContentsMargins(8, 0, 8, 0);
    m_root_layout->setSpacing(0);

    m_top_row = new QHBoxLayout();
    m_top_row->setContentsMargins(0, 0, 0, 0);
    m_top_row->setSpacing(4);

    m_bottom_row = new QHBoxLayout();
    m_bottom_row->setContentsMargins(0, 0, 0, 0);
    m_bottom_row->setSpacing(4);

    m_top_row->addWidget(m_icon_label);
    m_top_row->addWidget(m_title_label);
    m_top_row->addWidget(m_minimize_button);
    m_top_row->addWidget(m_maximize_button);
    m_top_row->addWidget(m_close_button);

    m_root_layout->addLayout(m_top_row);
    m_root_layout->addLayout(m_bottom_row);

    setLayout(m_root_layout);

    // Initial icon render based on default color/size and current window state.
    update_button_icons();

    connect(m_minimize_button, &QPushButton::clicked, this, &WindowTitleBar::minimize_requested);
    connect(m_maximize_button, &QPushButton::clicked, [this] {
        QWidget* top = window();
        const bool maximized = (top != nullptr) ? top->isMaximized() : false;
        if (maximized)
        {
            emit restore_requested();
        }
        else
        {
            emit maximize_requested();
        }
    });
    connect(m_close_button, &QPushButton::clicked, this, &WindowTitleBar::close_requested);
}

/**
 * @brief Sets the small icon displayed at the far left of the title bar.
 * @param icon The icon to display.
 */
auto WindowTitleBar::set_icon(const QIcon& icon) -> void
{
    if (m_icon_label != nullptr)
    {
        const int icon_size = 20;
        QPixmap pm = icon.pixmap(icon_size, icon_size);
        m_icon_label->setPixmap(pm);
        m_icon_label->setVisible(!pm.isNull());
    }
}

/**
 * @brief Sets the window title text.
 * @param title The title to display.
 */
auto WindowTitleBar::set_title(const QString& title) -> void
{
    if (m_title_label != nullptr)
    {
        m_title_label->setText(title);
    }
}

/**
 * @brief Set a menubar to be embedded in the title bar and re-parent it to this widget.
 * @param menubar Pointer to a QMenuBar to embed (may be nullptr to remove).
 */
auto WindowTitleBar::set_menubar(QMenuBar* menubar) -> void
{
    if (m_menu_bar != menubar)
    {
        if (m_menu_bar != nullptr)
        {
            m_menu_bar->setParent(nullptr);
            m_menu_bar->deleteLater();
        }

        m_menu_bar = menubar;

        if (m_menu_bar != nullptr)
        {
            m_menu_bar->setParent(this);
        }

        rebuild_layout();
    }
}

/**
 * @brief Choose the row where the menubar should be placed.
 * @param row Top to place between title and buttons; Bottom to place under icon+title.
 */
auto WindowTitleBar::set_menubar_row(RowPosition row) -> void
{
    if (m_menubar_row != row)
    {
        m_menubar_row = row;
        rebuild_layout();
    }
}

/**
 * @brief Set a custom widget to appear in the title bar and re-parent it to this widget.
 * @param widget Pointer to a QWidget to embed (may be nullptr to remove).
 */
auto WindowTitleBar::set_custom_widget(QWidget* widget) -> void
{
    if (m_custom_widget != widget)
    {
        if (m_custom_widget != nullptr)
        {
            m_custom_widget->setParent(nullptr);
            m_custom_widget->deleteLater();
        }

        m_custom_widget = widget;

        if (m_custom_widget != nullptr)
        {
            m_custom_widget->setParent(this);
        }

        rebuild_layout();
    }
}

/**
 * @brief Choose the row where the custom widget should be placed.
 * @param row Top to place between title/menu and buttons; Bottom to place under icon+title.
 */
auto WindowTitleBar::set_custom_widget_row(RowPosition row) -> void
{
    if (m_custom_row != row)
    {
        m_custom_row = row;
        rebuild_layout();
    }
}

/**
 * @brief Returns the embedded menubar if set.
 * @return QMenuBar* Pointer to the menubar or nullptr.
 */
[[nodiscard]] auto WindowTitleBar::get_menubar() const -> QMenuBar*
{
    QMenuBar* result = m_menu_bar;
    return result;
}

/**
 * @brief Returns the custom widget if set.
 * @return QWidget* Pointer to the custom widget or nullptr.
 */
[[nodiscard]] auto WindowTitleBar::get_custom_widget() const -> QWidget*
{
    QWidget* result = m_custom_widget;
    return result;
}

/**
 * @brief Returns the minimize button.
 * @return Pointer to the minimize QPushButton.
 */
auto WindowTitleBar::get_minimize_button() const -> QPushButton*
{
    QPushButton* result = m_minimize_button;
    return result;
}

/**
 * @brief Returns the maximize button.
 * @return Pointer to the maximize/restore QPushButton.
 */
auto WindowTitleBar::get_maximize_button() const -> QPushButton*
{
    QPushButton* result = m_maximize_button;
    return result;
}

/**
 * @brief Returns the close button.
 * @return Pointer to the close QPushButton.
 */
auto WindowTitleBar::get_close_button() const -> QPushButton*
{
    QPushButton* result = m_close_button;
    return result;
}

/**
 * @brief Returns the current color used to render window button SVG icons.
 * @return The current window button color.
 */
[[nodiscard]] auto WindowTitleBar::get_window_button_color() const -> QColor
{
    QColor result = m_window_button_color;
    return result;
}

/**
 * @brief Sets the color used to render window button SVG icons.
 * @param color The new color to apply to the icons.
 */
auto WindowTitleBar::set_window_button_color(const QColor& color) -> void
{
    if (m_window_button_color != color)
    {
        m_window_button_color = color;
        update_button_icons();
    }
}

/**
 * @brief Returns the current icon size in pixels for window buttons.
 * @return The icon size in pixels.
 */
[[nodiscard]] auto WindowTitleBar::get_window_button_icon_px() const -> int
{
    int result = m_window_button_icon_px;
    return result;
}

/**
 * @brief Sets the icon size in pixels for window buttons (square).
 * @param px The new icon size in pixels. Values < 1 are clamped to 1.
 */
auto WindowTitleBar::set_window_button_icon_px(int px) -> void
{
    int clamped = px;
    if (clamped < 1)
    {
        clamped = 1;
    }

    if (m_window_button_icon_px != clamped)
    {
        m_window_button_icon_px = clamped;
        apply_button_icon_size();
        update_button_icons();
    }
}

/**
 * @brief Handles mouse double-click events for maximize/restore.
 * @param event The mouse event.
 */
void WindowTitleBar::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_maximize_button->click();
    }
}

/**
 * @brief Handles mouse press events for drag start.
 * @param event The mouse event.
 */
void WindowTitleBar::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        emit drag_started(event->globalPos());
    }

    QWidget::mousePressEvent(event);
}

/**
 * @brief Install an event filter on the top-level window when shown.
 * @param event The show event.
 */
void WindowTitleBar::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);

    QWidget* top = window();
    if (top != nullptr)
    {
        top->installEventFilter(this);
        update_maximize_restore_icon();
    }
}

/**
 * @brief Intercepts top-level window state changes to update the max/restore icon.
 * @param watched The object being watched.
 * @param event The event being filtered.
 * @return true if handled; false otherwise.
 */
bool WindowTitleBar::eventFilter(QObject* watched, QEvent* event)
{
    bool handled = false;

    QWidget* top = window();
    if (top != nullptr)
    {
        if (watched == top)
        {
            if (event->type() == QEvent::WindowStateChange)
            {
                update_maximize_restore_icon();
                handled = false;
            }
        }
    }

    return handled;
}

/**
 * @brief Rebuild the layout to reflect current menubar/custom widget placement.
 *
 * Rules:
 *  - Top row order is [Icon][Title][MenuBar?][Custom?][...][Min][Max][Close].
 *  - Bottom row order is [MenuBar?][Custom?].
 *  - When only one of them is set to Bottom, it appears alone on the second row.
 */
auto WindowTitleBar::rebuild_layout() -> void
{
    if (m_top_row != nullptr)
    {
        for (int i = m_top_row->count() - 1; i >= 0; --i)
        {
            QWidget* w = m_top_row->itemAt(i)->widget();

            if (w != nullptr)
            {
                if (w == m_menu_bar || w == m_custom_widget)
                {
                    m_top_row->removeWidget(w);
                }
            }
        }
    }

    if (m_bottom_row != nullptr)
    {
        for (int i = m_bottom_row->count() - 1; i >= 0; --i)
        {
            QWidget* w = m_bottom_row->itemAt(i)->widget();

            if (w != nullptr)
            {
                if (w == m_menu_bar || w == m_custom_widget)
                {
                    m_bottom_row->removeWidget(w);
                }
            }
        }
    }

    if (m_bottom_row != nullptr)
    {
        if (m_menubar_row == RowPosition::Bottom && m_menu_bar != nullptr)
        {
            m_bottom_row->addWidget(m_menu_bar);
        }

        if (m_custom_row == RowPosition::Bottom && m_custom_widget != nullptr)
        {
            m_bottom_row->addWidget(m_custom_widget);
        }
    }

    if (m_top_row != nullptr)
    {
        int insert_index = m_top_row->count();

        for (int i = 0; i < m_top_row->count(); ++i)
        {
            QWidget* w = m_top_row->itemAt(i)->widget();

            if (w == m_minimize_button)
            {
                insert_index = i;
                i = m_top_row->count();
            }
        }

        if (m_menubar_row == RowPosition::Top && m_menu_bar != nullptr)
        {
            m_top_row->insertWidget(insert_index, m_menu_bar);
            insert_index = insert_index + 1;
        }

        if (m_custom_row == RowPosition::Top && m_custom_widget != nullptr)
        {
            m_top_row->insertWidget(insert_index, m_custom_widget);
            insert_index = insert_index + 1;
        }
    }
}

/**
 * @brief Apply the current icon size to all window buttons.
 */
auto WindowTitleBar::apply_button_icon_size() -> void
{
    const QSize sz(m_window_button_icon_px, m_window_button_icon_px);

    if (m_minimize_button != nullptr)
    {
        m_minimize_button->setIconSize(sz);
    }

    if (m_maximize_button != nullptr)
    {
        m_maximize_button->setIconSize(sz);
    }

    if (m_close_button != nullptr)
    {
        m_close_button->setIconSize(sz);
    }
}

/**
 * @brief Re-color and set all window button icons according to the current property.
 */
auto WindowTitleBar::update_button_icons() -> void
{
    const QSize sz(m_window_button_icon_px, m_window_button_icon_px);

    if (m_minimize_button != nullptr)
    {
        QIcon icon = QIcon(UiUtils::colored_svg_icon(":/Resources/Icons/titlebar-minimize.svg",
                                                     m_window_button_color, sz));
        m_minimize_button->setIcon(icon);
    }

    if (m_close_button != nullptr)
    {
        QIcon icon = QIcon(UiUtils::colored_svg_icon(":/Resources/Icons/titlebar-close.svg",
                                                     m_window_button_color, sz));
        m_close_button->setIcon(icon);
    }

    update_maximize_restore_icon();
}

/**
 * @brief Update the maximize/restore button icon based on the actual window state.
 */
auto WindowTitleBar::update_maximize_restore_icon() -> void
{
    if (m_maximize_button != nullptr)
    {
        QWidget* top = window();
        const bool maximized = (top != nullptr) ? top->isMaximized() : false;

        const QString path = maximized ? QString(":/Resources/Icons/titlebar-restore.svg")
                                       : QString(":/Resources/Icons/titlebar-maximize.svg");

        const QSize sz(m_window_button_icon_px, m_window_button_icon_px);
        QIcon icon = QIcon(UiUtils::colored_svg_icon(path, m_window_button_color, sz));
        m_maximize_button->setIcon(icon);
    }
}
