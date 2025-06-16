#pragma once

#include <QMainWindow>

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
         * @brief Pointer to the UI definition generated from the .ui file.
         */
        Ui::MainWindow* ui;
};
