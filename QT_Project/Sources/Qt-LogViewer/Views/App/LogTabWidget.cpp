#include "Qt-LogViewer/Views/App/LogTabWidget.h"

#include <QTabBar>

#include "Qt-LogViewer/Views/App/LogViewWidget.h"

/**
 * @brief Constructs a LogTabWidget object.
 * @param parent The parent widget, or nullptr.
 */
LogTabWidget::LogTabWidget(QWidget* parent): TabWidget(parent)
{
    setup_default_behavior();
}

/**
 * @brief Applies default tab behavior for log views.
 */
auto LogTabWidget::setup_default_behavior() -> void
{
    setTabsClosable(true);
    setElideMode(Qt::ElideRight);

    if (tabBar() != nullptr)
    {
        tabBar()->setMovable(true);
    }
}

/**
 * @brief Returns the LogViewWidget at a given tab index.
 * @param index The tab index.
 * @return Pointer to LogViewWidget if present; otherwise nullptr.
 */
auto LogTabWidget::log_view_at(int index) const -> LogViewWidget*
{
    LogViewWidget* result = nullptr;

    if (index >= 0 && index < count())
    {
        result = qobject_cast<LogViewWidget*>(widget(index));
    }

    return result;
}

/**
 * @brief Returns the current LogViewWidget.
 * @return Pointer to current LogViewWidget if present; otherwise nullptr.
 */
auto LogTabWidget::current_log_view() const -> LogViewWidget*
{
    LogViewWidget* result = log_view_at(currentIndex());

    return result;
}

/**
 * @brief Finds the tab index for a given view id.
 * @param view_id The view identifier.
 * @return Tab index if found; otherwise -1.
 */
auto LogTabWidget::find_view_index(const QUuid& view_id) const -> int
{
    int result = -1;
    bool tab_removed = false;
    const int tab_count = count();

    for (int i = 0; i < tab_count && !tab_removed; ++i)
    {
        auto* log_view_widget = log_view_at(i);

        if (log_view_widget != nullptr && log_view_widget->get_view_id() == view_id)
        {
            result = i;
            tab_removed = true;
        }
    }

    return result;
}

/**
 * @brief Sets file paths on a view tab identified by view id.
 * @param view_id The view identifier.
 * @param file_paths File paths to set on the target LogViewWidget.
 * @return True if the target tab was found and updated; otherwise false.
 */
auto LogTabWidget::set_view_file_paths(const QUuid& view_id,
                                       const QVector<QString>& file_paths) -> bool
{
    bool result = false;
    const int index = find_view_index(view_id);

    if (index >= 0)
    {
        LogViewWidget* log_view_widget = log_view_at(index);

        if (log_view_widget != nullptr)
        {
            log_view_widget->set_view_file_paths(file_paths);
            result = true;
        }
    }

    return result;
}

/**
 * @brief Removes a view tab by its view id.
 * @param view_id The view identifier.
 * @return True if a tab was removed; otherwise false.
 */
auto LogTabWidget::remove_view_tab_by_id(const QUuid& view_id) -> bool
{
    bool result = false;
    const int index = find_view_index(view_id);

    if (index >= 0)
    {
        removeTab(index);
        result = true;
    }

    return result;
}

/**
 * @brief Adds a LogViewWidget as tab, selects it, and optionally auto-resizes columns.
 * @param log_view_widget The view widget to add.
 * @param tab_title The tab title.
 * @param auto_resize_columns True to auto-resize after selecting.
 * @return New tab index, or -1 if input is invalid.
 */
auto LogTabWidget::add_log_view_tab(LogViewWidget* log_view_widget, const QString& tab_title,
                                    bool auto_resize_columns) -> int
{
    int result = -1;

    if (log_view_widget != nullptr)
    {
        result = addTab(log_view_widget, tab_title);
        setCurrentIndex(result);

        if (auto_resize_columns)
        {
            log_view_widget->auto_resize_columns();
        }
    }

    return result;
}

/**
 * @brief Removes all tabs.
 */
auto LogTabWidget::close_all_view_tabs() -> void
{
    while (count() > 0)
    {
        removeTab(0);
    }
}

/**
 * @brief Resizes columns of the current LogViewWidget table.
 */
auto LogTabWidget::auto_resize_current_columns() -> void
{
    LogViewWidget* log_view_widget = current_log_view();

    if (log_view_widget != nullptr)
    {
        log_view_widget->auto_resize_columns();
    }
}
