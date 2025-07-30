#include "Qt-LogViewer/Views/BaseMainWindow.h"

#include "Qt-LogViewer/Services/AppPreferencesInterface.h"
#include "Qt-LogViewer/Services/Settings.h"
#include "Qt-LogViewer/Services/StylesheetLoader.h"

/**
 * @brief Constructs a BaseMainWindow object.
 *
 * Initializes the main window, sets up settings and stylesheet loader, and creates the settings
 * file path.
 *
 * @param settings Optional pointer to a Settings object. If nullptr, a new Settings object is
 * created.
 * @param parent The parent widget, or nullptr if this is a top-level window.
 */
BaseMainWindow::BaseMainWindow(AppPreferencesInterface* preferences, QWidget* parent)
    : QMainWindow(parent),
      m_preferences(preferences),
      m_stylesheet_loader(new StylesheetLoader(this))
{
    qDebug() << "BaseMainWindow constructor started";

    // Note: Old SIGNAL/SLOT syntax is required here because the signal is declared as a pure
    // virtual function in the interface and not as a real Qt signal. The new function pointer
    // syntax only works with signals declared in QObject-based classes using Q_OBJECT.
    connect(dynamic_cast<QObject*>(m_preferences), SIGNAL(themeChanged(QString)), this,
            SLOT(onThemeChanged(QString)));

    qDebug() << "BaseMainWindow constructor finished";
}

/**
 * @brief Destructor for BaseMainWindow
 *
 * Cleans up resources and logs destruction.
 */
BaseMainWindow::~BaseMainWindow()
{
    qDebug() << "BaseMainWindow destructor called";
}

/**
 * @brief Gets the stylesheet loader object.
 *
 * This method provides access to the StylesheetLoader object used for managing application
 * stylesheets.
 *
 * @return Pointer to the StylesheetLoader object.
 */
auto BaseMainWindow::get_stylesheet_loader() const -> StylesheetLoader*
{
    return m_stylesheet_loader;
}

/**
 * @brief Saves the main window geometry, state, and window state to preferences.
 *
 * This method saves the current geometry, state, and window state of the main window to the
 * preferences object if it is not null.
 */
auto BaseMainWindow::save_window_settings() -> void
{
    if (m_preferences != nullptr)
    {
        m_preferences->set_mainwindow_geometry(saveGeometry());
        m_preferences->set_mainwindow_state(saveState());
        m_preferences->set_mainwindow_windowstate(windowState());
    }
    else
    {
        qWarning() << "[BaseMainWindow] Preferences object is null, cannot save window settings.";
    }
}

/**
 * @brief Restores the main window geometry, state, and window state from preferences.
 *
 * This method restores the main window's geometry, state, and window state from the preferences
 * object if it is not null. It checks if the geometry and state are not empty before restoring
 * them. If the window state is maximized, it shows the window maximized.
 */
auto BaseMainWindow::restore_window_settings() -> void
{
    if (m_preferences)
    {
        const QByteArray geometry = m_preferences->get_mainwindow_geometry();
        const QByteArray state = m_preferences->get_mainwindow_state();
        int window_state = m_preferences->get_mainwindow_windowstate();

        if (!geometry.isEmpty())
        {
            restoreGeometry(geometry);
        }

        if (!state.isEmpty())
        {
            restoreState(state);
        }

        if (window_state == Qt::WindowMaximized)
        {
            showMaximized();
        }
    }
    else
    {
        qWarning()
            << "[BaseMainWindow] Preferences object is null, cannot restore window settings.";
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
void BaseMainWindow::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);

    if (!m_window_restored)
    {
        restore_window_settings();
        m_window_restored = true;
    }

    if (!m_theme_applied && m_preferences != nullptr)
    {
        const QString theme = m_preferences->get_theme();
        m_stylesheet_loader->load_stylesheet(":/Resources/style.qss", theme);
        m_theme_applied = true;
        qInfo() << "[BaseMainWindow] Loaded theme from preferences (showEvent):" << theme;
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
void BaseMainWindow::closeEvent(QCloseEvent* event)
{
    save_window_settings();
    QMainWindow::closeEvent(event);
}

/**
 * @brief Slot: Handles theme changes.
 *
 * This slot is called when the application theme is changed. It updates the UI to reflect the new
 * theme.
 *
 * @param theme_name The name of the new theme (e.g. "Dark", "Light").
 */
void BaseMainWindow::onThemeChanged(const QString& theme_name)
{
    m_stylesheet_loader->load_stylesheet(":/Resources/style.qss", theme_name);
}
