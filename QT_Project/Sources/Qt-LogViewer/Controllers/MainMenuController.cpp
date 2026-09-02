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
#include <QSignalBlocker>
#include <QVariant>

#include "Qt-LogViewer/Models/RecentItemsModel.h"
#include "Qt-LogViewer/Models/RecentListSchema.h"

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
constexpr auto k_show_log_file_explorer_text =
    QT_TRANSLATE_NOOP("MainMenuController", "Show Log File Explorer");
constexpr auto k_show_log_details_text =
    QT_TRANSLATE_NOOP("MainMenuController", "Show Log Details");
constexpr auto k_show_log_level_pie_chart_text =
    QT_TRANSLATE_NOOP("MainMenuController", "Show Log Level Pie Chart");
constexpr auto k_settings_menu_text = QT_TRANSLATE_NOOP("MainMenuController", "&Settings");
constexpr auto k_settings_text = QT_TRANSLATE_NOOP("MainMenuController", "Settings...");
constexpr auto k_help_menu_text = QT_TRANSLATE_NOOP("MainMenuController", "&Help");
constexpr auto k_about_text = QT_TRANSLATE_NOOP("MainMenuController", "About %1");
constexpr auto k_about_qt_text = QT_TRANSLATE_NOOP("MainMenuController", "About Qt");
constexpr auto k_clear_recent_files_text =
    QT_TRANSLATE_NOOP("MainMenuController", "Clear Recent Files");

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
    : QObject(parent),
      m_menu_bar(menu_bar),
      m_recent_files_model(recent_files_model),
      m_recent_sessions_model(recent_sessions_model)
{
    initialize_menu();
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

        // Views menu
        m_action_show_log_file_explorer = new QAction(this);
        m_action_show_log_file_explorer->setCheckable(true);
        m_views_menu->addAction(m_action_show_log_file_explorer);

        m_action_show_log_details = new QAction(this);
        m_action_show_log_details->setCheckable(true);
        m_views_menu->addAction(m_action_show_log_details);

        m_action_show_log_level_pie_chart = new QAction(this);
        m_action_show_log_level_pie_chart->setCheckable(true);
        m_views_menu->addAction(m_action_show_log_level_pie_chart);

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
        connect(m_action_show_log_file_explorer, &QAction::toggled, this,
                &MainMenuController::show_log_file_explorer_toggled);
        connect(m_action_show_log_details, &QAction::toggled, this,
                &MainMenuController::show_log_details_toggled);
        connect(m_action_show_log_level_pie_chart, &QAction::toggled, this,
                &MainMenuController::show_log_level_pie_chart_toggled);
        connect(m_action_settings, &QAction::triggered, this,
                &MainMenuController::settings_requested);
        connect(m_action_about, &QAction::triggered, this, &MainMenuController::about_requested);
        connect(m_action_about_qt, &QAction::triggered, this,
                &MainMenuController::about_qt_requested);

        retranslate();
    }
}

/**
 * @brief Rebuilds the Recent Files and Recent Sessions submenus from models.
 *
 * Idempotent; clears and repopulates actions on each call.
 */
auto MainMenuController::rebuild_recent_menus() -> void
{
    if (m_recent_files_menu != nullptr && m_recent_files_model != nullptr)
    {
        m_recent_files_menu->clear();
        for (int row = 0; row < m_recent_files_model->rowCount(); ++row)
        {
            const QModelIndex index = m_recent_files_model->index(row, 0);
            const QString title =
                m_recent_files_model->data(index, to_role_id(RecentFileRole::FileName)).toString();
            const QString path =
                m_recent_files_model->data(index, to_role_id(RecentFileRole::FilePath)).toString();
            QAction* action = m_recent_files_menu->addAction(title);
            connect(action, &QAction::triggered, this,
                    [this, path]() { emit open_recent_file_requested(path); });
        }

        m_recent_files_menu->addSeparator();
        QAction* clear_action =
            m_recent_files_menu->addAction(translate(k_clear_recent_files_text));
        connect(clear_action, &QAction::triggered, this,
                &MainMenuController::clear_recent_files_requested);
    }

    if (m_recent_sessions_menu != nullptr && m_recent_sessions_model != nullptr)
    {
        m_recent_sessions_menu->clear();
        for (int row = 0; row < m_recent_sessions_model->rowCount(); ++row)
        {
            const QModelIndex index = m_recent_sessions_model->index(row, 0);
            const QString title =
                m_recent_sessions_model->data(index, to_role_id(RecentSessionRole::Name))
                    .toString();
            const QString id =
                m_recent_sessions_model->data(index, to_role_id(RecentSessionRole::Id)).toString();
            QAction* action = m_recent_sessions_menu->addAction(title);
            connect(action, &QAction::triggered, this,
                    [this, id]() { emit open_recent_session_requested(id); });
        }
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
        m_action_show_log_file_explorer->setText(translate(k_show_log_file_explorer_text));
        m_action_show_log_details->setText(translate(k_show_log_details_text));
        m_action_show_log_level_pie_chart->setText(translate(k_show_log_level_pie_chart_text));
        m_action_settings->setText(translate(k_settings_text));
        m_action_about->setText(translate(k_about_text).arg(QCoreApplication::applicationName()));
        m_action_about_qt->setText(translate(k_about_qt_text));
    }
}

/**
 * @brief Updates the enabled and checked state of all view actions without emitting toggled
 * signals.
 * @param enabled Whether view actions are enabled.
 * @param show_log_file_explorer Whether the log file explorer action is checked.
 * @param show_log_details Whether the log details action is checked.
 * @param show_log_level_pie_chart Whether the log level pie chart action is checked.
 */
auto MainMenuController::set_view_actions_state(bool enabled, bool show_log_file_explorer,
                                                bool show_log_details,
                                                bool show_log_level_pie_chart) -> void
{
    if (m_action_show_log_file_explorer != nullptr)
    {
        const QSignalBlocker explorer_blocker(m_action_show_log_file_explorer);
        const QSignalBlocker details_blocker(m_action_show_log_details);
        const QSignalBlocker pie_chart_blocker(m_action_show_log_level_pie_chart);

        m_action_show_log_file_explorer->setChecked(show_log_file_explorer);
        m_action_show_log_details->setChecked(show_log_details);
        m_action_show_log_level_pie_chart->setChecked(show_log_level_pie_chart);

        m_action_show_log_file_explorer->setEnabled(enabled);
        m_action_show_log_details->setEnabled(enabled);
        m_action_show_log_level_pie_chart->setEnabled(enabled);
    }
}
