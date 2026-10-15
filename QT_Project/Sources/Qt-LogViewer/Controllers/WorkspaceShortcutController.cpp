#include "Qt-LogViewer/Controllers/WorkspaceShortcutController.h"

#include <QAbstractItemView>
#include <QKeySequence>
#include <QShortcut>

#include "Qt-LogViewer/Controllers/LogPageCoordinator.h"
#include "Qt-LogViewer/Models/LogModel.h"
#include "Qt-LogViewer/Models/LogPageState.h"
#include "Qt-LogViewer/Views/App/LogDetailsWidget.h"
#include "Qt-LogViewer/Views/App/LogFilterBarWidget.h"
#include "Qt-LogViewer/Views/App/LogTabWidget.h"
#include "Qt-LogViewer/Views/App/LogTableView.h"
#include "Qt-LogViewer/Views/App/LogViewWidget.h"
#include "Qt-LogViewer/Views/App/SearchBarWidget.h"

/**
 * @file WorkspaceShortcutController.cpp
 * @brief Implements window-scoped keyboard coordination for the log workspace.
 */

/**
 * @brief Installs workspace shortcuts on the tab widget's window.
 * @param pages Pagination operations used for result navigation.
 * @param tab_widget Workspace tab container and shortcut host.
 * @param filter_bar Shared search controls.
 * @param details_widget Shared selected-entry inspector.
 * @param parent Optional QObject parent.
 */
WorkspaceShortcutController::WorkspaceShortcutController(LogPageCoordinator* pages,
                                                         LogTabWidget* tab_widget,
                                                         LogFilterBarWidget* filter_bar,
                                                         LogDetailsWidget* details_widget,
                                                         QObject* parent)
    : QObject(parent),
      m_pages(pages),
      m_tab_widget(tab_widget),
      m_filter_bar(filter_bar),
      m_details_widget(details_widget)
{
    setup_shortcuts();
}

/** @brief Installs the window-scoped keyboard shortcuts. */
auto WorkspaceShortcutController::setup_shortcuts() -> void
{
    if (m_tab_widget != nullptr)
    {
        /** @brief Creates one window-scoped shortcut on the workspace tab container. */
        const auto install_shortcut = [this](const QKeySequence& sequence,
                                             const QString& object_name, auto handler) {
            auto* shortcut = new QShortcut(sequence, m_tab_widget);
            shortcut->setContext(Qt::WindowShortcut);
            shortcut->setObjectName(object_name);
            connect(shortcut, &QShortcut::activated, this, handler);
        };

        install_shortcut(QKeySequence::Find, QStringLiteral("shortcutFocusSearch"), [this] {
            if (m_filter_bar != nullptr)
            {
                SearchBarWidget* search_bar = m_filter_bar->get_search_bar_widget();
                if (search_bar != nullptr && search_bar->isEnabled())
                {
                    search_bar->focus_search_input();
                }
            }
        });
        install_shortcut(
            QKeySequence(Qt::Key_Escape), QStringLiteral("shortcutClearSearch"), [this] {
                SearchBarWidget* search_bar =
                    m_filter_bar != nullptr ? m_filter_bar->get_search_bar_widget() : nullptr;
                if (search_bar != nullptr && !search_bar->get_search_text().isEmpty())
                {
                    search_bar->clear_search();
                }
                else
                {
                    focus_active_table();
                }
            });
        install_shortcut(QKeySequence(Qt::Key_F3), QStringLiteral("shortcutNextSearchResult"),
                         [this] { select_relative_result(1); });
        install_shortcut(QKeySequence(Qt::SHIFT | Qt::Key_F3),
                         QStringLiteral("shortcutPreviousSearchResult"),
                         [this] { select_relative_result(-1); });
        install_shortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_C),
                         QStringLiteral("shortcutCopyLogEntry"), [this] {
                             if (m_details_widget != nullptr)
                             {
                                 m_details_widget->copy_current_details();
                             }
                         });
        install_shortcut(QKeySequence(Qt::CTRL | Qt::Key_W),
                         QStringLiteral("shortcutCloseCurrentView"), [this] {
                             if (m_tab_widget != nullptr)
                             {
                                 m_tab_widget->close_current_tab();
                             }
                         });
        install_shortcut(QKeySequence(Qt::CTRL | Qt::Key_Tab),
                         QStringLiteral("shortcutNextViewTab"), [this] { switch_relative_tab(1); });
        install_shortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Tab),
                         QStringLiteral("shortcutPreviousViewTab"),
                         [this] { switch_relative_tab(-1); });
    }
}

/** @brief Focuses the active log table when one is available. */
auto WorkspaceShortcutController::focus_active_table() -> void
{
    LogViewWidget* log_view = m_tab_widget != nullptr ? m_tab_widget->current_log_view() : nullptr;
    LogTableView* table_view = log_view != nullptr ? log_view->get_table_view() : nullptr;
    if (table_view != nullptr)
    {
        table_view->setFocus(Qt::ShortcutFocusReason);
    }
}

/**
 * @brief Selects the next or previous filtered entry, including adjacent pages.
 * @param direction Positive for the next entry and negative for the previous entry.
 */
auto WorkspaceShortcutController::select_relative_result(int direction) -> void
{
    LogViewWidget* log_view = m_tab_widget != nullptr ? m_tab_widget->current_log_view() : nullptr;
    LogTableView* table_view = log_view != nullptr ? log_view->get_table_view() : nullptr;
    const QUuid view_id = log_view != nullptr ? log_view->get_view_id() : QUuid();
    const LogPageState* page_state =
        !view_id.isNull() && m_pages != nullptr ? m_pages->get_page_state(view_id) : nullptr;
    const bool has_search =
        m_filter_bar != nullptr && !m_filter_bar->get_search_text().trimmed().isEmpty();

    if (table_view != nullptr && table_view->model() != nullptr && page_state != nullptr &&
        direction != 0 && table_view->model()->rowCount() > 0 && has_search)
    {
        const QModelIndex current_index = table_view->currentIndex();
        int target_row = current_index.isValid()
                             ? current_index.row() + direction
                             : (direction > 0 ? 0 : table_view->model()->rowCount() - 1);
        qsizetype target_page = page_state->get_current_page();

        if (target_row >= table_view->model()->rowCount())
        {
            target_page = target_page < page_state->get_total_pages() ? target_page + 1 : 1;
            target_row = 0;
        }
        else if (target_row < 0)
        {
            target_page = target_page > 1 ? target_page - 1 : page_state->get_total_pages();
            target_row = -1;
        }

        if (target_page != page_state->get_current_page())
        {
            m_pages->set_current_page(view_id, target_page);
        }

        if (target_row < 0)
        {
            target_row = table_view->model()->rowCount() - 1;
        }

        const auto* log_model = qobject_cast<const LogModel*>(table_view->model());
        int target_column = log_model != nullptr ? log_model->find_column(LogField::Message) : 0;
        if (target_column < 0)
        {
            target_column = 0;
        }

        const QModelIndex target_index = table_view->model()->index(target_row, target_column);
        table_view->setCurrentIndex(target_index);
        table_view->scrollTo(target_index, QAbstractItemView::EnsureVisible);
        table_view->setFocus(Qt::ShortcutFocusReason);
    }
}

/**
 * @brief Activates the next or previous workspace tab with wraparound.
 * @param direction Positive for the next tab and negative for the previous tab.
 */
auto WorkspaceShortcutController::switch_relative_tab(int direction) -> void
{
    if (m_tab_widget != nullptr && m_tab_widget->count() > 1 && direction != 0)
    {
        const int tab_count = m_tab_widget->count();
        const int target_index =
            (m_tab_widget->currentIndex() + (direction > 0 ? 1 : tab_count - 1)) % tab_count;
        m_tab_widget->setCurrentIndex(target_index);
    }
}
