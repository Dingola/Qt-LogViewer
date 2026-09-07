#pragma once

#include <QObject>
#include <QString>

class QAction;
class QMenu;
class QMenuBar;
class RecentItemsModel;

/**
 * @file MainMenuController.h
 * @brief Controller responsible for the main menu bar and its actions.
 *
 * Responsibilities:
 * - Create and translate the main menu bar.
 * - Rebuild the Recent Files and Recent Sessions submenus from models.
 * - Add native dock toggle actions to the Views menu.
 * - Emit user intent without depending on MainWindow or LogViewerController.
 */
class MainMenuController: public QObject
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs a MainMenuController.
         * @param menu_bar The menu bar to populate.
         * @param recent_files_model Model containing recent log files.
         * @param recent_sessions_model Model containing recent sessions.
         * @param parent Optional QObject parent.
         */
        explicit MainMenuController(QMenuBar* menu_bar, RecentItemsModel* recent_files_model,
                                    RecentItemsModel* recent_sessions_model,
                                    QObject* parent = nullptr);

        /**
         * @brief Rebuilds the Recent Files and Recent Sessions submenus from models.
         *
         * Idempotent; clears and repopulates actions on each call.
         */
        auto rebuild_recent_menus() -> void;

        /**
         * @brief Retranslates all persistent menu and action texts in place.
         */
        auto retranslate() -> void;

        /**
         * @brief Adds a dock toggle action to the Views menu.
         * @param action Action supplied by QDockWidget::toggleViewAction().
         *
         * Null actions and actions that are already present are ignored.
         */
        auto add_view_action(QAction* action) -> void;

    signals:
        /** @brief Emitted when opening log files is requested. */
        void open_log_file_requested();

        /**
         * @brief Emitted when a recent log file is requested.
         * @param file_path Absolute path of the recent log file.
         */
        void open_recent_file_requested(const QString& file_path);

        /** @brief Emitted when clearing recent log files is requested. */
        void clear_recent_files_requested();

        /** @brief Emitted when saving the current session is requested. */
        void save_session_requested();

        /** @brief Emitted when opening a session through a dialog is requested. */
        void open_session_requested();

        /**
         * @brief Emitted when opening a recent session is requested.
         * @param session_id Identifier of the recent session.
         */
        void open_recent_session_requested(const QString& session_id);

        /** @brief Emitted when reopening the last session is requested. */
        void reopen_last_session_requested();

        /** @brief Emitted when opening the settings dialog is requested. */
        void settings_requested();

        /** @brief Emitted when showing application information is requested. */
        void about_requested();

        /** @brief Emitted when showing Qt information is requested. */
        void about_qt_requested();

        /** @brief Emitted when quitting the application is requested. */
        void quit_requested();

    private:
        /**
         * @brief Initializes the main menu bar and its actions.
         */
        auto initialize_menu() -> void;

    private:
        QMenuBar* m_menu_bar = nullptr;
        RecentItemsModel* m_recent_files_model = nullptr;
        RecentItemsModel* m_recent_sessions_model = nullptr;

        QMenu* m_file_menu = nullptr;
        QMenu* m_recent_files_menu = nullptr;
        QMenu* m_recent_sessions_menu = nullptr;
        QMenu* m_views_menu = nullptr;
        QMenu* m_settings_menu = nullptr;
        QMenu* m_help_menu = nullptr;

        QAction* m_action_open_log_file = nullptr;
        QAction* m_action_save_session = nullptr;
        QAction* m_action_open_session = nullptr;
        QAction* m_action_reopen_last_session = nullptr;
        QAction* m_action_quit = nullptr;
        QAction* m_action_settings = nullptr;
        QAction* m_action_about = nullptr;
        QAction* m_action_about_qt = nullptr;
};
