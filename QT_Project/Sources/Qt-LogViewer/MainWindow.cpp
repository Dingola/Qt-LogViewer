#include "Headers/Qt-LogViewer/MainWindow.h"

#include "ui_MainWindow.h"

/**
 * @brief Constructs a MainWindow object.
 *
 * Initializes the main window and sets up the user interface.
 *
 * @param parent The parent widget, or nullptr if this is a top-level window.
 */
MainWindow::MainWindow(QWidget* parent): QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

/**
 * @brief Destroys the MainWindow object.
 *
 * Cleans up any resources used by the main window.
 */
MainWindow::~MainWindow()
{
    delete ui;
}
