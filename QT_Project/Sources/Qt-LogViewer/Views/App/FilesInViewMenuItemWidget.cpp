#include "Qt-LogViewer/Views/App/FilesInViewMenuItemWidget.h"

#include <QEvent>
#include <QFileInfo>
#include <QFontMetrics>
#include <QStyle>

#include "Qt-LogViewer/Services/UiUtils.h"

/**
 * @file FilesInViewMenuItemWidget.cpp
 * @brief Implementation of FilesInViewMenuItemWidget with QSS-settable icon color/size and paths.
 */

/**
 * @brief Constructs the menu item widget.
 * @param parent Optional parent.
 */
FilesInViewMenuItemWidget::FilesInViewMenuItemWidget(QWidget* parent)
    : QWidget(parent),
      m_file_path(),
      m_icon_color(QColor("#666666")),
      m_icon_color_hover(QColor("#42a5f5")),
      m_icon_color_eye(),
      m_icon_color_eye_off(),
      m_icon_color_trash(),
      m_icon_px(14),
      m_icon_eye(QStringLiteral(":/Resources/Icons/eye.svg")),
      m_icon_eye_off(QStringLiteral(":/Resources/Icons/eye-off.svg")),
      m_icon_trash(QStringLiteral(":/Resources/Icons/trash.svg")),
      m_label(nullptr),
      m_btn_show_only(nullptr),
      m_btn_toggle_visibility(nullptr),
      m_btn_remove(nullptr),
      m_layout(nullptr)
{
    setup_ui();
}

/**
 * @brief Builds layout and wiring for controls.
 */
auto FilesInViewMenuItemWidget::setup_ui() -> void
{
    setContentsMargins(6, 2, 6, 2);

    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(6, 2, 6, 2);
    m_layout->setSpacing(6);

    m_label = new QLabel(this);
    m_label->setTextInteractionFlags(Qt::NoTextInteraction);
    m_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    m_btn_show_only = new QToolButton(this);
    m_btn_show_only->setProperty("variant", "inline-menu");
    m_btn_show_only->setAutoRaise(true);
    m_btn_show_only->setToolTip(tr("Show only this file"));
    m_btn_show_only->setCursor(Qt::PointingHandCursor);
    m_btn_show_only->setIconSize(QSize(m_icon_px, m_icon_px));
    m_btn_show_only->installEventFilter(this);

    m_btn_toggle_visibility = new QToolButton(this);
    m_btn_toggle_visibility->setProperty("variant", "inline-menu");
    m_btn_toggle_visibility->setAutoRaise(true);
    m_btn_toggle_visibility->setToolTip(tr("Hide this file"));
    m_btn_toggle_visibility->setCursor(Qt::PointingHandCursor);
    m_btn_toggle_visibility->setIconSize(QSize(m_icon_px, m_icon_px));
    m_btn_toggle_visibility->installEventFilter(this);

    m_btn_remove = new QToolButton(this);
    m_btn_remove->setProperty("variant", "inline-menu");
    m_btn_remove->setAutoRaise(true);
    m_btn_remove->setToolTip(tr("Remove this file from the current view"));
    m_btn_remove->setCursor(Qt::PointingHandCursor);
    m_btn_remove->setIconSize(QSize(m_icon_px, m_icon_px));
    m_btn_remove->installEventFilter(this);

    m_layout->addWidget(m_label, 1);
    m_layout->addWidget(m_btn_show_only, 0);
    m_layout->addWidget(m_btn_toggle_visibility, 0);
    m_layout->addWidget(m_btn_remove, 0);

    connect(m_btn_show_only, &QToolButton::clicked, this,
            [this]() { emit show_only_requested(m_file_path); });
    connect(m_btn_toggle_visibility, &QToolButton::clicked, this,
            [this]() { emit toggle_visibility_requested(m_file_path); });
    connect(m_btn_remove, &QToolButton::clicked, this,
            [this]() { emit remove_requested(m_file_path); });

    apply_icons();
    update_label_elided();
}

/**
 * @brief Sets the file path represented by this menu item and updates the label.
 * @param file_path Absolute or relative file path.
 */
auto FilesInViewMenuItemWidget::set_file_path(const QString& file_path) -> void
{
    m_file_path = file_path;
    update_label_elided();
}

/**
 * @brief Returns the file path represented by this menu item.
 * @return Full file path string.
 */
auto FilesInViewMenuItemWidget::get_file_path() const -> QString
{
    QString file_path_out = m_file_path;
    return file_path_out;
}

/**
 * @brief Updates the visibility toggle presentation and tooltip.
 * @param hidden True if file is currently hidden by filters.
 */
auto FilesInViewMenuItemWidget::set_hidden_state(bool hidden) -> void
{
    // Keep both actions enabled; this button toggles hide/show
    m_btn_toggle_visibility->setEnabled(true);
    m_btn_show_only->setEnabled(true);

    m_is_hidden_effective = hidden;
    m_btn_toggle_visibility->setToolTip(hidden ? tr("Show this file") : tr("Hide this file"));

    apply_icons();
}

/**
 * @brief Sets icon resource paths for the inline action buttons and reapplies icons.
 * @param eye Path to "Show Only" icon (eye).
 * @param eye_off Path to "Hide" icon (eye-off).
 * @param trash Path to "Remove" icon (trash).
 */
auto FilesInViewMenuItemWidget::set_icon_paths(const QString& eye, const QString& eye_off,
                                               const QString& trash) -> void
{
    m_icon_eye = eye;
    m_icon_eye_off = eye_off;
    m_icon_trash = trash;
    apply_icons();
}

/**
 * @brief Sets the base icon color for inline action buttons and reapplies icons.
 * @param color Base color.
 */
auto FilesInViewMenuItemWidget::set_icon_color(const QColor& color) -> void
{
    if (m_icon_color != color)
    {
        m_icon_color = color;
        apply_icons();
    }
}

/**
 * @brief Gets the base icon color for inline action buttons.
 * @return Current base color.
 */
auto FilesInViewMenuItemWidget::get_icon_color() const -> QColor
{
    QColor result = m_icon_color;
    return result;
}

/**
 * @brief Sets the hover/pressed icon color and reapplies icons when hovering.
 * @param color Hover color.
 */
auto FilesInViewMenuItemWidget::set_icon_color_hover(const QColor& color) -> void
{
    if (m_icon_color_hover != color)
    {
        m_icon_color_hover = color;
        apply_icons();
    }
}

/**
 * @brief Gets the hover/pressed icon color.
 * @return Current hover color.
 */
auto FilesInViewMenuItemWidget::get_icon_color_hover() const -> QColor
{
    QColor result = m_icon_color_hover;
    return result;
}

/**
 * @brief Sets the base color for the "eye" icon and reapplies icons.
 * @param color Eye icon base color.
 */
auto FilesInViewMenuItemWidget::set_icon_color_eye(const QColor& color) -> void
{
    if (m_icon_color_eye != color)
    {
        m_icon_color_eye = color;
        apply_icons();
    }
}

/**
 * @brief Gets the base color for the "eye" icon.
 * @return Current eye icon base color.
 */
auto FilesInViewMenuItemWidget::get_icon_color_eye() const -> QColor
{
    QColor result = m_icon_color_eye;
    return result;
}

/**
 * @brief Sets the base color for the "eye-off" icon and reapplies icons.
 * @param color Eye-off icon base color.
 */
auto FilesInViewMenuItemWidget::set_icon_color_eye_off(const QColor& color) -> void
{
    if (m_icon_color_eye_off != color)
    {
        m_icon_color_eye_off = color;
        apply_icons();
    }
}

/**
 * @brief Gets the base color for the "eye-off" icon.
 * @return Current eye-off icon base color.
 */
auto FilesInViewMenuItemWidget::get_icon_color_eye_off() const -> QColor
{
    QColor result = m_icon_color_eye_off;
    return result;
}

/**
 * @brief Sets the base color for the "trash" icon and reapplies icons.
 * @param color Trash icon base color.
 */
auto FilesInViewMenuItemWidget::set_icon_color_trash(const QColor& color) -> void
{
    if (m_icon_color_trash != color)
    {
        m_icon_color_trash = color;
        apply_icons();
    }
}

/**
 * @brief Gets the base color for the "trash" icon.
 * @return Current trash icon base color.
 */
auto FilesInViewMenuItemWidget::get_icon_color_trash() const -> QColor
{
    QColor result = m_icon_color_trash;
    return result;
}

/**
 * @brief Sets the icon size (square px) and reapplies icons.
 * @param px Icon size in pixels. Values < 1 are clamped to 1.
 */
auto FilesInViewMenuItemWidget::set_icon_px(int px) -> void
{
    int clamped = px;
    if (clamped < 1)
    {
        clamped = 1;
    }
    if (m_icon_px != clamped)
    {
        m_icon_px = clamped;
        const QSize size(m_icon_px, m_icon_px);
        m_btn_show_only->setIconSize(size);
        m_btn_toggle_visibility->setIconSize(size);
        m_btn_remove->setIconSize(size);
        apply_icons();
    }
}

/**
 * @brief Gets the icon size (square px).
 * @return Icon size in pixels.
 */
auto FilesInViewMenuItemWidget::get_icon_px() const -> int
{
    int result = m_icon_px;
    return result;
}

/**
 * @brief Sets the path to the "Show Only" icon and reapplies icons.
 * @param path Resource path to eye SVG.
 */
auto FilesInViewMenuItemWidget::set_icon_eye_path(const QString& path) -> void
{
    m_icon_eye = path;
    apply_icons();
}

/**
 * @brief Gets the path to the "Show Only" icon.
 * @return Resource path to eye SVG.
 */
auto FilesInViewMenuItemWidget::get_icon_eye_path() const -> QString
{
    QString result = m_icon_eye;
    return result;
}

/**
 * @brief Sets the path to the "Hide" icon and reapplies icons.
 * @param path Resource path to eye-off SVG.
 */
auto FilesInViewMenuItemWidget::set_icon_eye_off_path(const QString& path) -> void
{
    m_icon_eye_off = path;
    apply_icons();
}

/**
 * @brief Gets the path to the "Hide" icon.
 * @return Resource path to eye-off SVG.
 */
auto FilesInViewMenuItemWidget::get_icon_eye_off_path() const -> QString
{
    QString result = m_icon_eye_off;
    return result;
}

/**
 * @brief Sets the path to the "Remove" icon and reapplies icons.
 * @param path Resource path to trash SVG.
 */
auto FilesInViewMenuItemWidget::set_icon_trash_path(const QString& path) -> void
{
    m_icon_trash = path;
    apply_icons();
}

/**
 * @brief Gets the path to the "Remove" icon.
 * @return Resource path to trash SVG.
 */
auto FilesInViewMenuItemWidget::get_icon_trash_path() const -> QString
{
    QString result = m_icon_trash;
    return result;
}

/**
 * @brief Gets the pixels reserved on the right side (buttons/spacers).
 * @return Reserved pixel count.
 */
auto FilesInViewMenuItemWidget::get_label_reserved_px() const -> int
{
    int value = m_label_reserved_px;
    return value;
}

/**
 * @brief Sets the pixels reserved on the right side (buttons/spacers).
 * @param px Reserved pixel count (clamped to >= 0).
 */
auto FilesInViewMenuItemWidget::set_label_reserved_px(int px) -> void
{
    int clamped = (px < 0) ? 0 : px;
    if (m_label_reserved_px != clamped)
    {
        m_label_reserved_px = clamped;
        update_label_elided();
    }
}

/**
 * @brief Applies icons to inline buttons with the current color/size/paths.
 */
auto FilesInViewMenuItemWidget::apply_icons() -> void
{
    // Show Only uses a distinct icon
    update_button_icon(m_btn_show_only, m_icon_show_only, false);

    // Toggle visibility: eye when visible, eye-off when hidden
    const QString toggle_icon = m_is_hidden_effective ? m_icon_eye_off : m_icon_eye;
    update_button_icon(m_btn_toggle_visibility, toggle_icon, false);

    // Remove icon
    update_button_icon(m_btn_remove, m_icon_trash, false);
}

/**
 * @brief Updates a single button icon based on hover state.
 * @param btn Target button.
 * @param path SVG resource path.
 * @param hovered True to use hover color; false for base color.
 */
auto FilesInViewMenuItemWidget::update_button_icon(QToolButton* btn, const QString& path,
                                                   bool hovered) -> void
{
    if (btn != nullptr && !path.isEmpty())
    {
        // Select per-icon base color; fall back to generic icon_color
        QColor base_color = m_icon_color;

        if (btn == m_btn_show_only && m_icon_color_eye.isValid())
        {
            base_color = m_icon_color_eye;
        }
        else
        {
            if (btn == m_btn_toggle_visibility)
            {
                // Visibility toggle uses eye/eye-off; prefer dedicated eye color if set,
                // otherwise fall back to eye_off for hidden and generic color otherwise.
                if (m_is_hidden_effective && m_icon_color_eye_off.isValid())
                {
                    base_color = m_icon_color_eye_off;
                }
                else if (!m_is_hidden_effective && m_icon_color_eye.isValid())
                {
                    base_color = m_icon_color_eye;
                }
            }
            else
            {
                if (btn == m_btn_remove && m_icon_color_trash.isValid())
                {
                    base_color = m_icon_color_trash;
                }
            }
        }

        const QColor color = hovered ? m_icon_color_hover : base_color;
        btn->setIcon(make_icon(path, color));
    }
}

/**
 * @brief Builds a colored QIcon for an SVG at the requested color/size.
 * @param svg_path SVG resource path.
 * @param color Target color.
 * @return Colored QIcon.
 */
auto FilesInViewMenuItemWidget::make_icon(const QString& svg_path,
                                          const QColor& color) const -> QIcon
{
    QIcon icon = QIcon(UiUtils::colored_svg_icon(svg_path, color, QSize(m_icon_px, m_icon_px)));
    return icon;
}

/**
 * @brief Sets the path to the "Show Only" action icon and reapplies icons.
 * @param path Resource path to the show-only SVG.
 */
auto FilesInViewMenuItemWidget::set_icon_show_only_path(const QString& path) -> void
{
    m_icon_show_only = path;
    apply_icons();
}

/**
 * @brief Gets the path to the "Show Only" action icon.
 * @return Resource path to the show-only SVG.
 */
auto FilesInViewMenuItemWidget::get_icon_show_only_path() const -> QString
{
    QString result = m_icon_show_only;
    return result;
}

/**
 * @brief Handles hover/press events on inline buttons to swap icon color.
 * @param watched Watched object (buttons).
 * @param event The event to handle.
 * @return Always false; the event is not consumed.
 */
bool FilesInViewMenuItemWidget::eventFilter(QObject* watched, QEvent* event)
{
    bool handled = false;
    auto* button = qobject_cast<QToolButton*>(watched);

    if (button != nullptr)
    {
        const bool is_show_only = (button == m_btn_show_only);
        const bool is_toggle = (button == m_btn_toggle_visibility);
        const bool is_remove = (button == m_btn_remove);

        const bool enter = (event->type() == QEvent::Enter);
        const bool leave = (event->type() == QEvent::Leave);
        const bool press = (event->type() == QEvent::MouseButtonPress);
        const bool release = (event->type() == QEvent::MouseButtonRelease);

        if (enter || press || release || leave)
        {
            const bool hovered = (enter || press);

            if (is_show_only)
            {
                update_button_icon(m_btn_show_only, m_icon_show_only, hovered);
            }
            else
            {
                if (is_toggle)
                {
                    const QString toggle_icon = m_is_hidden_effective ? m_icon_eye_off : m_icon_eye;
                    update_button_icon(m_btn_toggle_visibility, toggle_icon, hovered);
                }
                else
                {
                    if (is_remove)
                    {
                        update_button_icon(m_btn_remove, m_icon_trash, hovered);
                    }
                }
            }
        }
    }
    return handled;
}

/**
 * @brief Re-elides label when the row width changes.
 */
void FilesInViewMenuItemWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    update_label_elided();
}

/**
 * @brief Updates the label with an elided file name and tooltip with full path.
 */
auto FilesInViewMenuItemWidget::update_label_elided() -> void
{
    QString display_text = m_file_path;

    QFileInfo info(m_file_path);
    if (info.exists())
    {
        display_text = info.fileName();
    }

    const int reserved = qMax(0, m_label_reserved_px);
    const int max_px = qMax(60, width() - reserved);

    QFontMetrics metrics(font());
    QString elided_text = metrics.elidedText(display_text, Qt::ElideMiddle, max_px);

    m_label->setText(elided_text);
    m_label->setToolTip(m_file_path);
    m_label->update();
}
