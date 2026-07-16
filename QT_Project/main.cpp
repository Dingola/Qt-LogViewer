/**
 * @file main.cpp
 * @brief This file contains the main function of a general Qt application template.
 */

#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QStandardPaths>
#include <memory>
#include <source_location>
#include <string>

#include "Qt-LogViewer/Models/LogFileInfo.h"
#include "Qt-LogViewer/Services/LogViewerSettings.h"
#include "Qt-LogViewer/Views/MainWindow.h"
#include "QtWidgetsCommonLib/Widgets/AppWindow.h"
#include "SimpleCppLogger/LogFormatter.h"
#include "SimpleCppLogger/LogLevel.h"
#include "SimpleCppLogger/LogMessage.h"
#include "SimpleCppLogger/Logger.h"
#include "SimpleQtLogger/QtFileAppender.h"
#include "SimpleQtLogger/QtLoggerAdapter.h"

namespace
{
class LogViewerFileFormatter final: public SimpleCppLogger::LogFormatter
{
    public:
        [[nodiscard]] auto format(const SimpleCppLogger::LogMessage& log_message,
                                  const std::source_location& location =
                                      std::source_location::current()) const -> std::string override
        {
            Q_UNUSED(location);

            QString message = QString::fromStdString(log_message.get_message());
            message.replace(QLatin1Char('\r'), QLatin1Char(' '));
            message.replace(QLatin1Char('\n'), QLatin1Char(' '));

            return QStringLiteral("%1 %2 %3 Qt-LogViewer")
                .arg(QDateTime::currentDateTime().toString(
                         QStringLiteral("yyyy-MM-dd HH:mm:ss.zzz")),
                     QString::fromStdString(SimpleCppLogger::to_string(log_message.get_level())),
                     message)
                .toStdString();
        }
};

[[nodiscard]] auto get_application_log_file_path() -> QString
{
    const QString log_directory =
        QDir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation))
            .filePath(QStringLiteral("logs"));

    QDir().mkpath(log_directory);

    return QDir(log_directory).filePath(QStringLiteral("Qt-LogViewer.log"));
}
}  // namespace

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

    auto file_formatter = std::make_shared<LogViewerFileFormatter>();
    auto file_appender = std::make_shared<SimpleQtLogger::QtFileAppender>(
        get_application_log_file_path(), file_formatter);

    auto& logger = SimpleCppLogger::Logger::get_instance();
    logger.add_appender(file_appender);
    logger.set_log_level(SimpleCppLogger::LogLevel::Debug);

    SimpleQtLogger::install_as_qt_message_handler();

    qInfo() << "Application log file:" << get_application_log_file_path();

    auto settings = LogViewerSettings(Settings::default_settings_file_path(), QSettings::IniFormat);

    auto* main_window = new MainWindow(&settings);
    QtWidgetsCommonLib::AppWindow app_window(nullptr, main_window);
    app_window.resize(1120, 800);
    app_window.set_app_title("Qt-LogViewer");
    app_window.set_app_icon(QIcon(":/Resources/Icons/App/AppIcon.svg"));
    app_window.set_adopt_menubar(true, QtWidgetsCommonLib::WindowTitleBar::RowPosition::Top);
    app_window.show();

    return app.exec();
}
