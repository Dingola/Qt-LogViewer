/**
 * @file main.cpp
 * @brief This file contains the main function of a general Qt application template.
 */

#include <QApplication>

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

    QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);

    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("QtApp"));
    app.setOrganizationName(QStringLiteral("QtTemplate"));
    app.setOrganizationDomain(QStringLiteral("AdrianHelbig.de"));

    // TODO: Add your main window or application logic here

    return app.exec();
}
