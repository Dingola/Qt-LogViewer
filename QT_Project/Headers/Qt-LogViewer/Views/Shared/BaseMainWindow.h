#pragma once

#include <QMainWindow>

class AppPreferencesInterface;
class StylesheetLoader;
class Translator;

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

        /**
         * @brief Gets the translator object.
         *
         * This method provides access to the Translator object used for managing translations in
         * the application.
         *
         * @return Pointer to the Translator object.
         */
        [[nodiscard]] auto get_translator() const -> Translator*;

    protected:
        /**
         * @brief Saves the main window geometry, state, and window state to preferences.
         */
        auto save_window_settings() -> void;

        /**
         * @brief Restores the main window geometry, state, and window state from preferences.
         */
        auto restore_window_settings() -> void;

        /**
         * @brief Handles the show event to apply the current theme.
         *
         * This method is called when the main window is shown. It applies the current theme if it
         * has not been applied yet.
         *
         * @param event The show event.
         */
        void showEvent(QShowEvent* event) override;

        /**
         * @brief Handles the close event to save window settings.
         *
         * This method is called when the main window is closed. It saves the current window
         * geometry, state, and window state to preferences.
         *
         * @param event The close event.
         */
        void closeEvent(QCloseEvent* event) override;

    private slots:
        /**
         * @brief Slot: Handles theme changes.
         *
         * This slot is called when the application theme is changed.
         * It updates the UI to reflect the new theme.
         * @param theme_name The name of the new theme (e.g. "Dark", "Light").
         */
        void onThemeChanged(const QString& theme_name);

        /**
         * @brief Slot: Handles language code changes.
         *
         * This slot is called when the application language code is changed.
         * It updates the UI to reflect the new language.
         * @param language_code The new language code (e.g. "en", "de").
         */
        void onLanguageCodeChanged(const QString& language_code);

        /**
         * @brief Slot: Handles language name changes.
         *
         * This slot is called when the application language name is changed.
         * It updates the UI to reflect the new language name.
         * @param language_name The new language name (e.g. "English", "Deutsch").
         */
        void onLanguageNameChanged(const QString& language_name);

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

        /**
         * @brief Indicates whether the window settings have been restored.
         */
        bool m_window_restored = false;

        /**
         * @brief Pointer to the Translator object used for managing translations.
         */
        Translator* m_translator = nullptr;

        /**
         * @brief Indicates whether the language has been applied.
         */
        bool m_language_applied = false;
};
