#include "Qt-LogViewer/Presenters/LogImportTabPresenter.h"

#include <QCoreApplication>
#include <optional>
#include <utility>

#include "Qt-LogViewer/Controllers/LogImportCoordinator.h"
#include "Qt-LogViewer/Controllers/SessionController.h"
#include "Qt-LogViewer/Models/LogFileInfo.h"
#include "Qt-LogViewer/Presenters/WorkspacePresenter.h"
#include "Qt-LogViewer/Services/LogPreviewService.h"
#include "Qt-LogViewer/Services/LogViewerSettings.h"
#include "Qt-LogViewer/Views/App/LogImportWidget.h"
#include "Qt-LogViewer/Views/App/LogTabWidget.h"
#include "Qt-LogViewer/Views/App/LogViewWidget.h"

/**
 * @file LogImportTabPresenter.cpp
 * @brief Implements the workflow for temporary parsing-profile import tabs.
 */

namespace
{
constexpr qsizetype k_import_batch_size = 1000;
constexpr int k_status_message_timeout_ms = 3000;
constexpr auto k_untitled_session_text = QT_TRANSLATE_NOOP("MainWindow", "Untitled Session");
}  // namespace

/**
 * @brief Constructs the import-tab workflow from its focused collaborators.
 * @param settings Parsing-profile persistence used by the import widget.
 * @param preview_service Bounded, side-effect-free preview parser.
 * @param import_coordinator Confirmed asynchronous import workflow.
 * @param tab_widget Tab container receiving temporary and completed views.
 * @param log_view_factory Factory for the completed log-view widget.
 * @param session_controller Optional session workflow for newly created views.
 * @param workspace_presenter Optional shared workspace presentation coordinator.
 * @param parent Optional QObject parent.
 */
LogImportTabPresenter::LogImportTabPresenter(
    LogViewerSettings& settings, const LogPreviewService& preview_service,
    LogImportCoordinator& import_coordinator, LogTabWidget& tab_widget,
    LogViewFactory log_view_factory, SessionController* session_controller,
    WorkspacePresenter* workspace_presenter, QObject* parent)
    : QObject(parent),
      m_settings(settings),
      m_preview_service(preview_service),
      m_import_coordinator(import_coordinator),
      m_tab_widget(tab_widget),
      m_log_view_factory(std::move(log_view_factory)),
      m_session_controller(session_controller),
      m_workspace_presenter(workspace_presenter)
{}

/**
 * @brief Opens a temporary profile-selection tab for one file.
 * @param log_file_info File selected for import.
 * @param target_view Existing target view, or a null identifier for a new view.
 */
auto LogImportTabPresenter::show_import_tab(const LogFileInfo& log_file_info,
                                            const QUuid& target_view) -> void
{
    auto* import_widget = new LogImportWidget(log_file_info.get_file_path(), m_settings,
                                              m_preview_service, &m_tab_widget);
    const QString import_title =
        QCoreApplication::translate("MainWindow", "Import %1").arg(log_file_info.get_file_name());
    const int import_tab_index = m_tab_widget.addTab(import_widget, import_title);
    m_tab_widget.setCurrentIndex(import_tab_index);

    connect(import_widget, &LogImportWidget::cancel_requested, this,
            [this, import_widget] { close_import_tab(import_widget); });
    connect(import_widget, &LogImportWidget::import_requested, this,
            [this, import_widget, log_file_info, target_view] {
                if (target_view.isNull())
                {
                    import_into_new_view(import_widget, log_file_info);
                }
                else
                {
                    import_into_existing_view(import_widget, log_file_info, target_view);
                }
            });
}

/**
 * @brief Removes and schedules deletion of an import widget.
 * @param import_widget Temporary widget to close.
 */
auto LogImportTabPresenter::close_import_tab(LogImportWidget* import_widget) -> void
{
    const int tab_index = m_tab_widget.indexOf(import_widget);

    if (tab_index >= 0)
    {
        m_tab_widget.removeTab(tab_index);
        import_widget->deleteLater();
    }
}

/**
 * @brief Replaces a confirmed import tab with a newly created log view.
 * @param import_widget Confirmed import widget.
 * @param log_file_info File selected for import.
 */
auto LogImportTabPresenter::import_into_new_view(LogImportWidget* import_widget,
                                                 const LogFileInfo& log_file_info) -> void
{
    const std::optional<LogParsingProfile> selected_profile = import_widget->get_selected_profile();
    const int import_tab_index = m_tab_widget.indexOf(import_widget);

    if (selected_profile.has_value() && import_tab_index >= 0)
    {
        QString session_id;
        if (m_session_controller != nullptr)
        {
            session_id = m_session_controller->ensure_current_session(
                QCoreApplication::translate("MainWindow", k_untitled_session_text));
        }

        const QUuid view_id = m_import_coordinator.import_file_async(
            log_file_info.get_file_path(), selected_profile.value(), k_import_batch_size);
        LogViewWidget* log_view_widget =
            !view_id.isNull() && m_log_view_factory ? m_log_view_factory(view_id) : nullptr;

        if (log_view_widget != nullptr)
        {
            m_tab_widget.removeTab(import_tab_index);
            const int log_tab_index = m_tab_widget.insertTab(import_tab_index, log_view_widget,
                                                             log_file_info.get_file_name());
            m_tab_widget.setCurrentIndex(log_tab_index);
            log_view_widget->auto_resize_columns();
            import_widget->deleteLater();

            if (m_session_controller != nullptr && !session_id.isEmpty())
            {
                m_session_controller->request_expand_session(session_id);
            }

            if (m_workspace_presenter != nullptr)
            {
                m_workspace_presenter->refresh_active_view();
                m_workspace_presenter->refresh_start_page();
            }
        }
        else
        {
            emit status_message_requested(tr("The log view could not be created."),
                                          k_status_message_timeout_ms);
        }
    }
}

/**
 * @brief Submits a confirmed import to an existing view when its tab still exists.
 * @param import_widget Confirmed import widget.
 * @param log_file_info File selected for import.
 * @param target_view Intended target view.
 */
auto LogImportTabPresenter::import_into_existing_view(LogImportWidget* import_widget,
                                                      const LogFileInfo& log_file_info,
                                                      const QUuid& target_view) -> void
{
    const std::optional<LogParsingProfile> selected_profile = import_widget->get_selected_profile();
    const int import_tab_index = m_tab_widget.indexOf(import_widget);
    const int target_tab_index = m_tab_widget.find_view_index(target_view);

    if (selected_profile.has_value() && import_tab_index >= 0 && target_tab_index >= 0)
    {
        close_import_tab(import_widget);
        m_tab_widget.setCurrentIndex(m_tab_widget.find_view_index(target_view));

        const bool enqueued =
            m_import_coordinator.import_file_async(target_view, log_file_info.get_file_path(),
                                                   selected_profile.value(), k_import_batch_size);
        const QString message =
            enqueued ? QCoreApplication::translate("MainWindow", "Queued file for current view: %1")
                           .arg(log_file_info.get_file_name())
                     : QCoreApplication::translate("MainWindow",
                                                   "File already present in current view: %1")
                           .arg(log_file_info.get_file_name());
        emit status_message_requested(message, k_status_message_timeout_ms);
    }
    else if (selected_profile.has_value() && import_tab_index >= 0)
    {
        emit status_message_requested(
            QCoreApplication::translate("MainWindow", "The target view is no longer available."),
            k_status_message_timeout_ms);
    }
}
