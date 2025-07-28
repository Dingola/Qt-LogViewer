#pragma once

#include <QAction>
#include <QDockWidget>
#include <QMainWindow>
#include <QModelIndex>
#include <QPlainTextEdit>

#include "Qt-LogViewer/Controllers/LogViewerController.h"
#include "Qt-LogViewer/Services/AppSettings.h"
#include "Qt-LogViewer/Services/StylesheetLoader.h"

namespace Ui
{
class MainWindow;
}

/**
 * @class MainWindow
 * @brief The main application window class.
 *
 * This class represents the main window of the application, providing
 * the central user interface and handling main window events.
 */
class MainWindow: public QMainWindow
{
        Q_OBJECT

    public:
        /**
         * @brief Constructs a MainWindow object.
         *
         * Initializes the main window and its user interface.
         *
         * @param parent The parent widget, or nullptr if this is a top-level window.
         */
        explicit MainWindow(QWidget* parent = nullptr);

        /**
         * @brief Destroys the MainWindow object.
         *
         * Cleans up any resources used by the main window.
         */
        ~MainWindow() override;

    private:
        /**
         * @brief Initializes the main menu bar and its actions.
         */
        auto initialize_menu() -> void;

        /**
         * @brief Updates the log level filter in the controller.
         */
        auto update_level_filter() -> void;

        /**
         * @brief Updates the search filter in the controller.
         */
        auto update_search_filter() -> void;

        /**
         * @brief Updates the log details view when a row is selected.
         * @param current The current selected index.
         */
        auto update_log_details(const QModelIndex& current) -> void;

        /**
         * @brief Updates the application combo box with the given application names.
         * @param app_names The set of application names to populate the combo box.
         */
        auto update_app_combo_box(const QSet<QString>& app_names) -> void;

        /**
         * @brief Updates the pagination widget based on the current page and total pages.
         *
         * This method retrieves the current page and total pages from the proxy model,
         * ensures they are within valid ranges, and updates the pagination widget accordingly.
         */
        auto update_pagination_widget() -> void;

        /**
         * @brief Loads log files and updates the UI.
         * @param files The list of log file paths to load.
         */
        auto load_files_and_update_ui(const QStringList& files) -> void;

        /**
         * @brief Handles drag enter events to allow dropping log files.
         * @param event The drag enter event.
         */
        void dragEnterEvent(QDragEnterEvent* event) override;

        /**
         * @brief Handles drop events to load log files via drag and drop.
         * @param event The drop event.
         */
        void dropEvent(QDropEvent* event) override;

        /**
         * @brief Handles resize events to adjust the layout.
         * @param event The resize event.
         */
        void resizeEvent(QResizeEvent* event) override;

    private slots:
        /**
         * @brief Opens log files using a file dialog.
         */
        void open_log_files();

        /**
         * @brief Shows the settings dialog for changing application settings.
         */
        void show_settings_dialog();

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
         * @brief Pointer to the UI definition generated from the .ui file.
         */
        Ui::MainWindow* ui;

        /**
         * @brief Controller for log loading and filtering.
         */
        LogViewerController* m_controller = nullptr;

        /**
         * @brief Action for opening log files (appears in the File menu).
         */
        QAction* m_action_open_log_file = nullptr;

        /**
         * @brief Action for opening log files (appears in the Settings menu).
         */
        QAction* m_action_settings = nullptr;

        /**
         * @brief Dock widget for displaying log details.
         */
        QDockWidget* m_log_details_dock_widget = nullptr;

        /**
         * @brief Text edit widget for showing detailed log information.
         */
        QPlainTextEdit* m_log_details_text_edit = nullptr;

        /**
         * @brief Loader for application stylesheets, allowing dynamic loading and variable
         * substitution.
         */
        StylesheetLoader* m_stylesheet_loader;

        /**
         * @brief Pointer to the application settings service.
         *
         * This is used to access and modify application-specific settings.
         */
        AppSettings* m_app_settings = nullptr;
};
