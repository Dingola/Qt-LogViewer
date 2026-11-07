/**
 * @file MainMenuController.cpp
 * @brief Implements MainMenuController for coordinating the main menu bar.
 */

#include "Qt-LogViewer/Controllers/MainMenuController.h"

#include <QAction>
#include <QCoreApplication>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>
#include <QModelIndex>
#include <QVariant>

#include "Qt-LogViewer/Models/RecentItemsModel.h"
#include "Qt-LogViewer/Models/RecentRoles.h"
#include "Qt-LogViewer/Views/Shared/RecentItemsMenuBinder.h"

namespace
{
constexpr auto k_translation_context = "MainMenuController";
constexpr auto k_file_menu_text = QT_TRANSLATE_NOOP("MainMenuController", "&File");
constexpr auto k_open_log_file_text = QT_TRANSLATE_NOOP("MainMenuController", "Open Log File...");
constexpr auto k_recent_files_text = QT_TRANSLATE_NOOP("MainMenuController", "Recent Files");
constexpr auto k_recent_sessions_text = QT_TRANSLATE_NOOP("MainMenuController", "Recent Sessions");
constexpr auto k_save_session_text = QT_TRANSLATE_NOOP("MainMenuController", "Save Session...");
constexpr auto k_open_session_text = QT_TRANSLATE_NOOP("MainMenuController", "Open Session...");
constexpr auto k_reopen_last_session_text =
    QT_TRANSLATE_NOOP("MainMenuController", "Reopen Last Session");
constexpr auto k_quit_text = QT_TRANSLATE_NOOP("MainMenuController", "&Quit");
constexpr auto k_views_menu_text = QT_TRANSLATE_NOOP("MainMenuController", "&Views");
constexpr auto k_settings_menu_text = QT_TRANSLATE_NOOP("MainMenuController", "&Settings");
constexpr auto k_settings_text = QT_TRANSLATE_NOOP("MainMenuController", "Settings...");
constexpr auto k_help_menu_text = QT_TRANSLATE_NOOP("MainMenuController", "&Help");
constexpr auto k_about_text = QT_TRANSLATE_NOOP("MainMenuController", "About %1");
constexpr auto k_about_qt_text = QT_TRANSLATE_NOOP("MainMenuController", "About Qt");
constexpr auto k_clear_recent_files_text =
    QT_TRANSLATE_NOOP("MainMenuController", "Clear Recent Files");
constexpr auto k_remove_unavailable_files_text =
    QT_TRANSLATE_NOOP("MainMenuController", "Remove Unavailable Files...");
constexpr auto k_unavailable_recent_file_text = QT_TRANSLATE_NOOP("MainMenuController", "%1 — %2");
constexpr auto k_no_recent_files_text = QT_TRANSLATE_NOOP("MainMenuController", "No Recent Files");
constexpr auto k_no_recent_sessions_text =
    QT_TRANSLATE_NOOP("MainMenuController", "No Recent Sessions");

/**
 * @brief Translates menu text using the MainMenuController translation context.
 * @param source_text Source text registered in the translation catalog.
 * @return Translated text.
 */
[[nodiscard]] auto translate(const char* source_text) -> QString
{
    return QCoreApplication::translate(k_translation_context, source_text);
}
}  // namespace

/**
 * @brief Constructs a MainMenuController.
 * @param menu_bar The menu bar to populate.
 * @param recent_files_model Model containing recent log files.
 * @param recent_sessions_model Model containing recent sessions.
 * @param parent Optional QObject parent.
 */
MainMenuController::MainMenuController(QMenuBar* menu_bar, RecentItemsModel* recent_files_model,
                                       RecentItemsModel* recent_sessions_model, QObject* parent)
    : QObject(parent), m_menu_bar(menu_bar)
{
    if (m_menu_bar != nullptr)
    {
        initialize_menu();

        const RecentItemsMenuRoles roles{
            to_role_id(RecentItemRole::Title),   to_role_id(RecentItemRole::Identity),
            to_role_id(RecentItemRole::Enabled), to_role_id(RecentItemRole::ToolTip),
            to_role_id(RecentItemRole::Icon),    to_role_id(RecentItemRole::StatusText)};

        m_action_remove_unavailable_files = new QAction(this);
        connect(m_action_remove_unavailable_files, &QAction::triggered, this,
                &MainMenuController::remove_unavailable_recent_files_requested);

        m_recent_files_binder =
            new RecentItemsMenuBinder(m_recent_files_menu, recent_files_model, roles, this);
        m_recent_files_binder->set_footer_actions({m_action_remove_unavailable_files});
        connect(m_recent_files_binder, &RecentItemsMenuBinder::item_activated, this,
                [this](const QVariant& identity) {
                    emit open_recent_file_requested(identity.toString());
                });
        connect(m_recent_files_binder, &RecentItemsMenuBinder::clear_requested, this,
                &MainMenuController::clear_recent_files_requested);
        connect(m_recent_files_binder, &RecentItemsMenuBinder::contains_disabled_items_changed,
                m_action_remove_unavailable_files, &QAction::setEnabled);

        m_recent_sessions_binder =
            new RecentItemsMenuBinder(m_recent_sessions_menu, recent_sessions_model, roles, this);
        connect(m_recent_sessions_binder, &RecentItemsMenuBinder::item_activated, this,
                [this](const QVariant& identity) {
                    emit open_recent_session_requested(identity.toString());
                });

        retranslate();
    }
}

/**
 * @brief Initializes the main menu bar and its actions.
 */
auto MainMenuController::initialize_menu() -> void
{
    if (m_menu_bar != nullptr)
    {
        // Main menus and recent submenus
        m_file_menu = new QMenu(m_menu_bar);
        m_recent_files_menu = new QMenu(m_file_menu);
        m_recent_sessions_menu = new QMenu(m_file_menu);
        m_views_menu = new QMenu(m_menu_bar);
        m_settings_menu = new QMenu(m_menu_bar);
        m_help_menu = new QMenu(m_menu_bar);

        // File menu: open log files and recent items
        m_action_open_log_file = new QAction(this);
        m_action_open_log_file->setShortcut(QKeySequence::Open);
        m_file_menu->addAction(m_action_open_log_file);
        m_file_menu->addMenu(m_recent_files_menu);
        m_file_menu->addMenu(m_recent_sessions_menu);

        // File menu: session actions
        m_action_save_session = new QAction(this);
        m_action_open_session = new QAction(this);
        m_action_reopen_last_session = new QAction(this);
        m_file_menu->addSeparator();
        m_file_menu->addAction(m_action_save_session);
        m_file_menu->addAction(m_action_open_session);
        m_file_menu->addAction(m_action_reopen_last_session);

        // File menu: quit application
        m_action_quit = new QAction(this);
#ifdef Q_OS_WIN
        m_action_quit->setShortcut(QKeySequence(QStringLiteral("Ctrl+Q")));
#else
        m_action_quit->setShortcut(QKeySequence::Quit);
#endif
        m_file_menu->addSeparator();
        m_file_menu->addAction(m_action_quit);

        // Views menu actions are added from registered dock widgets

        // Settings menu
        m_action_settings = new QAction(this);
        m_action_settings->setShortcut(QKeySequence(QStringLiteral("Ctrl+,")));
        m_settings_menu->addAction(m_action_settings);

        // Help menu
        m_action_about = new QAction(this);
        m_action_about_qt = new QAction(this);
        m_help_menu->addAction(m_action_about);
        m_help_menu->addAction(m_action_about_qt);

        // Add menus in their display order
        m_menu_bar->addMenu(m_file_menu);
        m_menu_bar->addMenu(m_views_menu);
        m_menu_bar->addMenu(m_settings_menu);
        m_menu_bar->addMenu(m_help_menu);

        // Connect persistent actions to user-intent signals
        connect(m_action_open_log_file, &QAction::triggered, this,
                &MainMenuController::open_log_file_requested);
        connect(m_action_save_session, &QAction::triggered, this,
                &MainMenuController::save_session_requested);
        connect(m_action_open_session, &QAction::triggered, this,
                &MainMenuController::open_session_requested);
        connect(m_action_reopen_last_session, &QAction::triggered, this,
                &MainMenuController::reopen_last_session_requested);
        connect(m_action_quit, &QAction::triggered, this, &MainMenuController::quit_requested);
        connect(m_action_settings, &QAction::triggered, this,
                &MainMenuController::settings_requested);
        connect(m_action_about, &QAction::triggered, this, &MainMenuController::about_requested);
        connect(m_action_about_qt, &QAction::triggered, this,
                &MainMenuController::about_qt_requested);
        connect(m_recent_files_menu, &QMenu::aboutToShow, this,
                &MainMenuController::recent_files_refresh_requested);
    }
}

/**
 * @brief Rebuilds the Recent Files and Recent Sessions submenus from models.
 *
 * Idempotent; clears and repopulates actions on each call.
 */
auto MainMenuController::rebuild_recent_menus() -> void
{
    if (m_recent_files_binder != nullptr)
    {
        m_recent_files_binder->refresh();
    }

    if (m_recent_sessions_binder != nullptr)
    {
        m_recent_sessions_binder->refresh();
    }
}

/**
 * @brief Retranslates all persistent menu and action texts in place.
 */
auto MainMenuController::retranslate() -> void
{
    if (m_file_menu != nullptr)
    {
        m_file_menu->setTitle(translate(k_file_menu_text));
        m_recent_files_menu->setTitle(translate(k_recent_files_text));
        m_recent_sessions_menu->setTitle(translate(k_recent_sessions_text));
        m_views_menu->setTitle(translate(k_views_menu_text));
        m_settings_menu->setTitle(translate(k_settings_menu_text));
        m_help_menu->setTitle(translate(k_help_menu_text));

        m_action_open_log_file->setText(translate(k_open_log_file_text));
        m_action_save_session->setText(translate(k_save_session_text));
        m_action_open_session->setText(translate(k_open_session_text));
        m_action_reopen_last_session->setText(translate(k_reopen_last_session_text));
        m_action_quit->setText(translate(k_quit_text));
        m_action_settings->setText(translate(k_settings_text));
        m_action_about->setText(translate(k_about_text).arg(QCoreApplication::applicationName()));
        m_action_about_qt->setText(translate(k_about_qt_text));
        m_action_remove_unavailable_files->setText(translate(k_remove_unavailable_files_text));

        m_recent_files_binder->set_empty_text(translate(k_no_recent_files_text));
        m_recent_files_binder->set_clear_text(translate(k_clear_recent_files_text));
        m_recent_files_binder->set_disabled_text_format(translate(k_unavailable_recent_file_text));
        m_recent_sessions_binder->set_empty_text(translate(k_no_recent_sessions_text));
    }
}

/**
 * @brief Adds a dock toggle action to the Views menu.
 * @param action Action supplied by QDockWidget::toggleViewAction().
 */
auto MainMenuController::add_view_action(QAction* action) -> void
{
    if (m_views_menu != nullptr && action != nullptr && !m_views_menu->actions().contains(action))
    {
        m_views_menu->addAction(action);
    }
}
