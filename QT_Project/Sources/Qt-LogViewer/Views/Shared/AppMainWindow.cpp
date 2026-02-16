#include "Qt-LogViewer/Views/Shared/AppMainWindow.h"

#include "Qt-LogViewer/Services/Preferences/IUiPreferences.h"
#include "QtWidgetsCommonLib/Services/Translator.h"
#include "QtWidgetsCommonLib/Utils/StylesheetLoader.h"

using QtWidgetsCommonLib::StylesheetLoader;
using QtWidgetsCommonLib::Translator;

/**
 * @brief Constructs a AppMainWindow object.
 *
 * Initializes the main window, sets up settings and stylesheet loader, and creates the settings
 * file path.
 *
 * @param settings Optional pointer to a Settings object. If nullptr, a new Settings object is
 * created.
 * @param parent The parent widget, or nullptr if this is a top-level window.
 */
AppMainWindow::AppMainWindow(IUiPreferences* ui_preferences, QWidget* parent)
    : QMainWindow(parent),
      m_ui_preferences(ui_preferences),
      m_stylesheet_loader(new StylesheetLoader(this)),
      m_translator(new Translator(this))
{
    qDebug() << "AppMainWindow constructor started";

    // Note: Old SIGNAL/SLOT syntax is required here because the signal is declared as a pure
    // virtual function in the interface and not as a real Qt signal. The new function pointer
    // syntax only works with signals declared in QObject-based classes using Q_OBJECT.
    connect(dynamic_cast<QObject*>(m_ui_preferences), SIGNAL(themeChanged(QString)), this,
            SLOT(onThemeChanged(QString)));
    connect(dynamic_cast<QObject*>(m_ui_preferences), SIGNAL(languageNameChanged(QString)), this,
            SLOT(onLanguageNameChanged(QString)));
    connect(dynamic_cast<QObject*>(m_ui_preferences), SIGNAL(languageCodeChanged(QString)), this,
            SLOT(onLanguageCodeChanged(QString)));

    qDebug() << "AppMainWindow constructor finished";
}

/**
 * @brief Destructor for AppMainWindow
 *
 * Cleans up resources and logs destruction.
 */
AppMainWindow::~AppMainWindow()
{
    qDebug() << "AppMainWindow destructor called";
}

/**
 * @brief Gets the stylesheet loader object.
 *
 * This method provides access to the StylesheetLoader object used for managing application
 * stylesheets.
 *
 * @return Pointer to the StylesheetLoader object.
 */
auto AppMainWindow::get_stylesheet_loader() const -> StylesheetLoader*
{
    return m_stylesheet_loader;
}

/**
 * @brief Gets the translator object.
 *
 * This method provides access to the Translator object used for managing translations in the
 * application.
 *
 * @return Pointer to the Translator object.
 */
auto AppMainWindow::get_translator() const -> Translator*
{
    return m_translator;
}

/**
 * @brief Saves the main window geometry, state, and window state to preferences.
 *
 * This method saves the current geometry, state, and window state of the main window to the
 * preferences object if it is not null.
 */
auto AppMainWindow::save_window_settings() -> void
{
    if (m_ui_preferences != nullptr)
    {
        QWidget* tlw = topLevelWidget();
        QByteArray geometry;
        int wnd_state = 0;

        if (tlw != nullptr && tlw != this)
        {
            geometry = tlw->saveGeometry();
            wnd_state = static_cast<int>(tlw->windowState());
        }
        else
        {
            geometry = saveGeometry();
            wnd_state = static_cast<int>(windowState());
        }

        m_ui_preferences->set_mainwindow_geometry(geometry);
        m_ui_preferences->set_mainwindow_state(saveState());
        m_ui_preferences->set_mainwindow_windowstate(wnd_state);
    }
    else
    {
        qWarning() << "[AppMainWindow] Preferences object is null, cannot save window settings.";
    }
}

/**
 * @brief Restores the main window geometry, state, and window state from preferences.
 *
 * This method restores the main window's geometry, state, and window state from the preferences
 * object if it is not null. It checks if the geometry and state are not empty before restoring
 * them. If the window state is maximized, it shows the window maximized.
 */
auto AppMainWindow::restore_window_settings() -> void
{
    if (m_ui_preferences != nullptr)
    {
        const QByteArray geometry = m_ui_preferences->get_mainwindow_geometry();
        const QByteArray state = m_ui_preferences->get_mainwindow_state();
        int window_state = m_ui_preferences->get_mainwindow_windowstate();

        QWidget* tlw = topLevelWidget();

        if (!geometry.isEmpty())
        {
            if (tlw != nullptr && tlw != this)
            {
                tlw->restoreGeometry(geometry);
            }
            else
            {
                restoreGeometry(geometry);
            }
        }

        if (!state.isEmpty())
        {
            restoreState(state);
        }

        if (window_state == Qt::WindowMaximized)
        {
            if (tlw != nullptr && tlw != this)
            {
                tlw->showMaximized();
            }
            else
            {
                showMaximized();
            }
        }
    }
    else
    {
        qWarning() << "[AppMainWindow] Preferences object is null, cannot restore window settings.";
    }
}

/**
 * @brief Handles the show event of the main window.
 *
 * This method is called when the main window is shown. It checks if the theme has been applied
 * and loads the stylesheet based on the user's preferences.
 *
 * @param event The show event.
 */
void AppMainWindow::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);

    if (!m_window_restored)
    {
        restore_window_settings();
        m_window_restored = true;
    }

    if (m_ui_preferences != nullptr)
    {
        if (!m_theme_applied)
        {
            const QString theme = m_ui_preferences->get_theme();
            m_stylesheet_loader->load_stylesheet(
                "D:/Projects/VS_Projects/Qt-LogViewer/QT_Project/Resources/style.qss", theme);
            m_theme_applied = true;
            qInfo() << "[AppMainWindow] Loaded theme from preferences (showEvent):" << theme;
        }

        if (!m_language_applied)
        {
            const QString language_code = m_ui_preferences->get_language_code();
            m_translator->load_translation(language_code);
            m_language_applied = true;
            qInfo() << "[AppMainWindow] Loaded language from preferences (showEvent):"
                    << language_code;
        }
    }
}

/**
 * @brief Handles the close event of the main window.
 *
 * This method is called when the main window is closed. It saves the current window geometry,
 * state, and window state to preferences.
 *
 * @param event The close event.
 */
void AppMainWindow::closeEvent(QCloseEvent* event)
{
    save_window_settings();
    QMainWindow::closeEvent(event);
}

/**
 * @brief Slot: Handles language code changes.
 *
 * This slot is called when the application language code is changed. It can be used to update the
 * UI or perform other actions based on the new language code.
 *
 * @param language_code The new language code (e.g. "en", "de").
 */
void AppMainWindow::onLanguageCodeChanged(const QString& language_code)
{
    QMap<QString, QString> code_name_map = m_translator->get_language_code_name_map();
    QString language_name = code_name_map.value(language_code);

    if (!language_name.isEmpty())
    {
        if (m_ui_preferences != nullptr && m_ui_preferences->get_language_name() != language_name)
        {
            auto obj = dynamic_cast<QObject*>(m_ui_preferences);
            obj->blockSignals(true);
            m_ui_preferences->set_language_name(language_name);
            obj->blockSignals(false);
        }
    }
    else
    {
        qWarning() << "[AppMainWindow] Language name for language code not found:" << language_code;
    }

    m_translator->load_translation(language_code);
}

/**
 * @brief Slot: Handles language name changes.
 *
 * This slot is called when the application language name is changed. It can be used to update the
 * UI or perform other actions based on the new language name.
 *
 * @param language_name The new language name (e.g. "English", "Deutsch").
 */
void AppMainWindow::onLanguageNameChanged(const QString& language_name)
{
    QMap<QString, QString> code_name_map = m_translator->get_language_code_name_map();
    QString found_code;

    for (auto it = code_name_map.begin(); it != code_name_map.end(); ++it)
    {
        if (it.value().compare(language_name, Qt::CaseInsensitive) == 0)
        {
            found_code = it.key();
            break;
        }
    }

    if (!found_code.isEmpty())
    {
        if (m_ui_preferences != nullptr && m_ui_preferences->get_language_code() != found_code)
        {
            auto obj = dynamic_cast<QObject*>(m_ui_preferences);
            obj->blockSignals(true);
            m_ui_preferences->set_language_code(found_code);
            obj->blockSignals(false);
        }

        m_translator->load_translation(found_code);
    }
    else
    {
        qWarning() << "[AppMainWindow] Language name not found:" << language_name;
    }
}

/**
 * @brief Slot: Handles theme changes.
 *
 * This slot is called when the application theme is changed. It updates the UI to reflect the new
 * theme.
 *
 * @param theme_name The name of the new theme (e.g. "Dark", "Light").
 */
void AppMainWindow::onThemeChanged(const QString& theme_name)
{
    m_stylesheet_loader->load_stylesheet(":/Resources/style.qss", theme_name);
}
