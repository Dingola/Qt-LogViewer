/**
 * @file LogViewerApplicationTest.cpp
 * @brief Verifies startup and shutdown of the composed Qt-LogViewer application.
 */

#include "Qt-LogViewer/Application/LogViewerApplicationTest.h"

#include <QApplication>
#include <QIcon>
#include <QSettings>
#include <QTemporaryDir>

#include "Qt-LogViewer/Application/LogViewerApplication.h"
#include "Qt-LogViewer/Services/LogViewerSettings.h"
#include "QtWidgetsCommonLib/Widgets/AppWindow.h"

namespace
{
/**
 * @brief Finds the currently displayed outer Qt-LogViewer application window.
 * @return Application window, or nullptr when none exists.
 */
[[nodiscard]] auto find_app_window() -> QtWidgetsCommonLib::AppWindow*
{
    QtWidgetsCommonLib::AppWindow* result = nullptr;

    for (QWidget* widget: QApplication::topLevelWidgets())
    {
        auto* candidate = qobject_cast<QtWidgetsCommonLib::AppWindow*>(widget);
        if (candidate != nullptr)
        {
            result = candidate;
        }
    }

    return result;
}
}  // namespace

/**
 * @brief Captures global application styling before the complete UI applies its theme.
 */
void LogViewerApplicationTest::SetUp()
{
    m_original_stylesheet = qApp->styleSheet();
}

/**
 * @brief Restores global application styling for subsequently executed widget tests.
 */
void LogViewerApplicationTest::TearDown()
{
    qApp->setStyleSheet(m_original_stylesheet);
    QApplication::processEvents();
}

/**
 * @brief Ensures the application composition can show and destroy its window safely.
 */
TEST_F(LogViewerApplicationTest, StartsAndStopsWithExplicitDependencies)
{
    QTemporaryDir temporary_directory;
    ASSERT_TRUE(temporary_directory.isValid());

    LogViewerSettings settings(temporary_directory.filePath(QStringLiteral("settings.ini")),
                               QSettings::IniFormat);

    {
        LogViewerApplication application(settings);

        QtWidgetsCommonLib::AppWindow* app_window = find_app_window();
        ASSERT_NE(app_window, nullptr);
        EXPECT_EQ(app_window->size(), QSize(1120, 800));
        EXPECT_EQ(app_window->windowTitle(), QStringLiteral("Qt-LogViewer"));
        EXPECT_TRUE(app_window->get_adopt_menubar());

        application.show();
        QApplication::processEvents();
        EXPECT_TRUE(app_window->isVisible());
    }
}

/**
 * @brief Ensures callers can override application-level window settings during startup.
 */
TEST_F(LogViewerApplicationTest, StartsWithCustomWindowOptions)
{
    QTemporaryDir temporary_directory;
    ASSERT_TRUE(temporary_directory.isValid());

    LogViewerSettings settings(temporary_directory.filePath(QStringLiteral("settings.ini")),
                               QSettings::IniFormat);
    LogViewerApplicationOptions options;
    options.initial_window_size = QSize(1280, 720);
    options.window_title = QStringLiteral("Configured Qt-LogViewer");
    options.window_icon_path = QStringLiteral(":/Resources/Icons/settings.svg");
    options.adopt_menubar = false;

    {
        LogViewerApplication application(settings, options);

        QtWidgetsCommonLib::AppWindow* app_window = find_app_window();
        ASSERT_NE(app_window, nullptr);
        EXPECT_EQ(app_window->size(), options.initial_window_size);
        EXPECT_EQ(app_window->windowTitle(), options.window_title);
        const QImage actual_icon = app_window->windowIcon().pixmap(32, 32).toImage();
        const QImage expected_icon = QIcon(options.window_icon_path).pixmap(32, 32).toImage();
        EXPECT_EQ(actual_icon, expected_icon);
        EXPECT_EQ(app_window->get_adopt_menubar(), options.adopt_menubar);

        application.show();
        QApplication::processEvents();
        EXPECT_TRUE(app_window->isVisible());
    }
}
