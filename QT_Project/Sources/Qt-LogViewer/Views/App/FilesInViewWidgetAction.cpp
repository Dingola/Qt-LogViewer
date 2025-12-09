#include "Qt-LogViewer/Views/App/FilesInViewWidgetAction.h"

#include <QPointer>

#include "Qt-LogViewer/Views/App/FilesInViewMenuItemWidget.h"

/**
 * @file FilesInViewWidgetAction.cpp
 * @brief Implementation of FilesInViewWidgetAction. Hosts FilesInViewMenuItemWidget in menus.
 */

/**
 * @brief Constructs the action.
 * @param parent Optional parent object.
 */
FilesInViewWidgetAction::FilesInViewWidgetAction(QObject* parent)
    : QWidgetAction(parent),
      m_file_path(),
      m_icon_eye(),
      m_icon_eye_off(),
      m_icon_trash(),
      m_icon_color(QColor("#666666")),
      m_icon_color_hover(QColor("#42a5f5")),
      m_icon_px(14),
      m_hidden_effective(false)
{}

/**
 * @brief Sets the file path carried by this action's embedded widget.
 * @param file_path File path to forward to the embedded widget.
 */
auto FilesInViewWidgetAction::set_file_path(const QString& file_path) -> void
{
    m_file_path = file_path;
}

/**
 * @brief Returns the file path carried by this action's embedded widget.
 * @return File path string.
 */
auto FilesInViewWidgetAction::get_file_path() const -> QString
{
    QString file_path_out = m_file_path;
    return file_path_out;
}

/**
 * @brief Sets icon resource paths (optional; defaults are provided).
 * @param eye Path to "Show Only" icon.
 * @param eye_off Path to "Hide" icon.
 * @param trash Path to "Remove" icon.
 */
auto FilesInViewWidgetAction::set_icon_paths(const QString& eye, const QString& eye_off,
                                             const QString& trash) -> void
{
    m_icon_eye = eye;
    m_icon_eye_off = eye_off;
    m_icon_trash = trash;
}

/**
 * @brief Sets the base icon color for inline buttons.
 * @param color Base color.
 */
auto FilesInViewWidgetAction::set_icon_color(const QColor& color) -> void
{
    m_icon_color = color;
}

/**
 * @brief Gets the base icon color for inline buttons.
 * @return Current base color.
 */
auto FilesInViewWidgetAction::get_icon_color() const -> QColor
{
    QColor result = m_icon_color;
    return result;
}

/**
 * @brief Sets the hover/pressed icon color for inline buttons.
 * @param color Hover color.
 */
auto FilesInViewWidgetAction::set_icon_color_hover(const QColor& color) -> void
{
    m_icon_color_hover = color;
}

/**
 * @brief Gets the hover/pressed icon color for inline buttons.
 * @return Current hover color.
 */
auto FilesInViewWidgetAction::get_icon_color_hover() const -> QColor
{
    QColor result = m_icon_color_hover;
    return result;
}

/**
 * @brief Sets the icon size in pixels (square).
 * @param px Icon size in pixels. Values < 1 are clamped to 1.
 */
auto FilesInViewWidgetAction::set_icon_px(int px) -> void
{
    int clamped = px;
    if (clamped < 1)
    {
        clamped = 1;
    }
    m_icon_px = clamped;
}

/**
 * @brief Gets the icon size in pixels (square).
 * @return Icon size in pixels.
 */
auto FilesInViewWidgetAction::get_icon_px() const -> int
{
    int result = m_icon_px;
    return result;
}

/**
 * @brief Sets whether the file represented by this action is effectively hidden.
 * @param hidden True if effectively hidden; false otherwise.
 */
auto FilesInViewWidgetAction::set_hidden_effective(bool hidden) -> void
{
    m_hidden_effective = hidden;

    const QList<QWidget*> widgets = createdWidgets();
    for (QWidget* w: widgets)
    {
        auto* row = qobject_cast<FilesInViewMenuItemWidget*>(w);
        if (row != nullptr)
        {
            row->set_hidden_state(m_hidden_effective);
        }
    }
}

/**
 * @brief Returns whether the file represented by this action is effectively hidden.
 * @return True if effectively hidden; false otherwise.
 */
auto FilesInViewWidgetAction::get_hidden_effective() const -> bool
{
    bool result = m_hidden_effective;
    return result;
}

/**
 * @brief Creates the embedded widget instance and wires its signals to this action.
 * @param parent Menu or parent widget.
 * @return Newly created widget hosted by the QWidgetAction.
 */
QWidget* FilesInViewWidgetAction::createWidget(QWidget* parent)
{
    auto* item_widget = new FilesInViewMenuItemWidget(parent);
    item_widget->set_file_path(m_file_path);

    // Apply resource paths (overrides or defaults under :/Resources/Icons)
    const QString eye_path =
        m_icon_eye.isEmpty() ? QStringLiteral(":/Resources/Icons/eye.svg") : m_icon_eye;
    const QString eye_off_path =
        m_icon_eye_off.isEmpty() ? QStringLiteral(":/Resources/Icons/eye-off.svg") : m_icon_eye_off;
    const QString trash_path =
        m_icon_trash.isEmpty() ? QStringLiteral(":/Resources/Icons/trash.svg") : m_icon_trash;

    item_widget->set_icon_paths(eye_path, eye_off_path, trash_path);

    // Apply coloring and size (QSS may override via qproperty-* on the widget)
    item_widget->set_icon_color(m_icon_color);
    item_widget->set_icon_color_hover(m_icon_color_hover);
    item_widget->set_icon_px(m_icon_px);

    // Forward effective hidden state to toggle Hide/Show presentation
    item_widget->set_hidden_state(m_hidden_effective);

    // Forward signals
    connect(item_widget, &FilesInViewMenuItemWidget::show_only_requested, this,
            [this](const QString& path) { emit show_only_requested(path); });
    connect(item_widget, &FilesInViewMenuItemWidget::toggle_visibility_requested, this,
            [this](const QString& path) { emit toggle_visibility_requested(path); });
    connect(item_widget, &FilesInViewMenuItemWidget::remove_requested, this,
            [this](const QString& path) { emit remove_requested(path); });

    return item_widget;
}

/**
 * @brief Deletes the embedded widget safely using deleteLater().
 * @param widget Widget instance to delete (may be nullptr).
 */
auto FilesInViewWidgetAction::deleteWidget(QWidget* widget) -> void
{
    if (widget != nullptr)
    {
        widget->deleteLater();
    }
}
