#pragma once

#include <QObject>
#include <QString>
#include <QUuid>
#include <functional>

class LogFileInfo;
class LogImportCoordinator;
class LogImportWidget;
class LogPreviewService;
class LogTabWidget;
class LogViewWidget;
class LogViewerSettings;
class SessionController;
class WorkspacePresenter;

/**
 * @file LogImportTabPresenter.h
 * @brief Declares the workflow for temporary parsing-profile import tabs.
 */

/**
 * @class LogImportTabPresenter
 * @brief Coordinates profile selection, cancellation, and confirmed imports for tabbed views.
 *
 * The presenter owns no application state. Preview parsing is supplied directly to the import
 * widget, while confirmed work is submitted to LogImportCoordinator. A focused factory keeps
 * construction and binding of the resulting LogViewWidget at the window boundary.
 */
class LogImportTabPresenter final: public QObject
{
        Q_OBJECT

    public:
        /** @brief Factory creating a bound log-view widget for a newly registered view. */
        using LogViewFactory = std::function<LogViewWidget*(const QUuid&)>;

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
        explicit LogImportTabPresenter(LogViewerSettings& settings,
                                       const LogPreviewService& preview_service,
                                       LogImportCoordinator& import_coordinator,
                                       LogTabWidget& tab_widget, LogViewFactory log_view_factory,
                                       SessionController* session_controller,
                                       WorkspacePresenter* workspace_presenter,
                                       QObject* parent = nullptr);

        /**
         * @brief Opens a temporary profile-selection tab for one file.
         * @param log_file_info File selected for import.
         * @param target_view Existing target view, or a null identifier for a new view.
         */
        auto show_import_tab(const LogFileInfo& log_file_info,
                             const QUuid& target_view = QUuid()) -> void;

    signals:
        /**
         * @brief Requests a transient message from the hosting window.
         * @param message Text to display.
         * @param timeout_ms Display duration in milliseconds.
         */
        void status_message_requested(const QString& message, int timeout_ms);

    private:
        /**
         * @brief Removes and schedules deletion of an import widget.
         * @param import_widget Temporary widget to close.
         */
        auto close_import_tab(LogImportWidget* import_widget) -> void;

        /**
         * @brief Replaces a confirmed import tab with a newly created log view.
         * @param import_widget Confirmed import widget.
         * @param log_file_info File selected for import.
         */
        auto import_into_new_view(LogImportWidget* import_widget,
                                  const LogFileInfo& log_file_info) -> void;

        /**
         * @brief Submits a confirmed import to an existing view when its tab still exists.
         * @param import_widget Confirmed import widget.
         * @param log_file_info File selected for import.
         * @param target_view Intended target view.
         */
        auto import_into_existing_view(LogImportWidget* import_widget,
                                       const LogFileInfo& log_file_info,
                                       const QUuid& target_view) -> void;

    private:
        LogViewerSettings& m_settings;
        const LogPreviewService& m_preview_service;
        LogImportCoordinator& m_import_coordinator;
        LogTabWidget& m_tab_widget;
        LogViewFactory m_log_view_factory;
        SessionController* m_session_controller = nullptr;
        WorkspacePresenter* m_workspace_presenter = nullptr;
};
