/**
 * @file main.cpp
 * @brief This file contains the main function of a general Qt application template.
 */

#include <QApplication>

#include "Qt-LogViewer/Services/LogViewerSettings.h"
#include "Qt-LogViewer/Views/MainWindow.h"
#include "Qt-LogViewer/Views/Shared/AppWindow.h"
#include "SimpleQtLogger/QtLoggerAdapter.h"

/**
 * @brief The main function initializes the Qt application and executes the application event loop.
 *
 * @param argc The number of command-line arguments.
 * @param argv The command-line arguments.
 *
 * @return The exit code of the application.
 */
auto main(int argc, char* argv[]) -> int
{
    // This line ensures that the resources are included in the final application binary when using
    // static linking.
    Q_INIT_RESOURCE(resources);

    qRegisterMetaType<LogFileInfo>("LogFileInfo");

    QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);

    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("Qt-LogViewer"));
    app.setOrganizationName(QStringLiteral("AdrianHelbig"));
    app.setOrganizationDomain(QStringLiteral("AdrianHelbig.de"));

    SimpleQtLogger::install_as_qt_message_handler();

    auto settings = LogViewerSettings(Settings::default_settings_file_path(), QSettings::IniFormat);

    auto* main_window = new MainWindow(&settings);
    AppWindow app_window(nullptr, main_window);
    app_window.resize(1120, 800);
    app_window.set_app_icon(QIcon(":/Resources/Icons/App/AppIcon.svg"));
    app_window.set_adopt_menubar(true, WindowTitleBar::RowPosition::Top);
    app_window.show();

    return app.exec();
}
