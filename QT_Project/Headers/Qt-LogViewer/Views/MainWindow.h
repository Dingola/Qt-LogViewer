#pragma once

#include <QAction>
#include <QMainWindow>
#include <QModelIndex>

#include "Qt-LogViewer/Controllers/LogViewerController.h"

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

    private slots:
        /**
         * @brief Opens log files using a file dialog.
         */
        void open_log_files();

    private:
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
};
