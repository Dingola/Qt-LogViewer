#pragma once

#include <QMainWindow>

class AppPreferencesInterface;
class StylesheetLoader;

/**
 * @class BaseMainWindow
 * @brief Base class for main windows in the application.
 *
 * This class provides basic functionality for managing application settings and stylesheets.
 */
class BaseMainWindow: public QMainWindow
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs a BaseMainWindow object.
         *
         * Initializes the main window, settings, and stylesheet loader.
         *
         * @param settings Optional pointer to a Settings object. If nullptr, a new Settings object
         * is created.
         * @param parent The parent widget, or nullptr if this is a top-level window.
         */
        explicit BaseMainWindow(AppPreferencesInterface* preferences = nullptr,
                                QWidget* parent = nullptr);

        /**
         * @brief Destroys the BaseMainWindow object.
         *
         * Cleans up any resources used by the main window.
         */
        ~BaseMainWindow() override;

        /**
         * @brief Gets the stylesheet loader object.
         *
         * This method provides access to the StylesheetLoader object used for managing application
         * stylesheets.
         *
         * @return Pointer to the StylesheetLoader object.
         */
        [[nodiscard]] auto get_stylesheet_loader() const -> StylesheetLoader*;

    protected:
        /**
         * @brief Handles the show event to apply the current theme.
         *
         * This method is called when the main window is shown. It applies the current theme if it
         * has not been applied yet.
         *
         * @param event The show event.
         */
        void showEvent(QShowEvent* event) override;

    private slots:
        /**
         * @brief Slot: Handles theme changes.
         *
         * This slot is called when the application theme is changed.
         * It updates the UI to reflect the new theme.
         * @param theme_name The name of the new theme (e.g. "Dark", "Light").
         */
        void onThemeChanged(const QString& theme_name);

    private:
        /**
         * @brief Pointer to the Settings object used for managing application settings.
         */
        AppPreferencesInterface* m_preferences;

        /**
         * @brief Pointer to the StylesheetLoader object used for managing application stylesheets.
         */
        StylesheetLoader* m_stylesheet_loader;

        /**
         * @brief Indicates whether the theme has been applied.
         */
        bool m_theme_applied = false;
};
