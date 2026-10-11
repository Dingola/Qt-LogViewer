/**
 * @file ApplicationStoragePathsTest.cpp
 * @brief Verifies the canonical application storage layout.
 */

#include "Qt-LogViewer/Services/ApplicationStoragePathsTest.h"

#include <QDir>
#include <QFileInfo>
#include <QLineEdit>
#include <QListWidget>
#include <QSettings>
#include <QStandardPaths>
#include <QToolButton>

#include "Qt-LogViewer/Services/ApplicationStoragePaths.h"
#include "Qt-LogViewer/Services/LogViewerSettings.h"
#include "Qt-LogViewer/Views/App/Dialogs/SettingsDialog.h"

/** @test Verifies that the default layout uses the application configuration root exactly once. */
TEST_F(ApplicationStoragePathsTest, UsesApplicationConfigurationRootWithoutDuplicateAppFolder)
{
    const ApplicationStoragePaths paths;
    const QString expected_root =
        QDir(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)).absolutePath();

    EXPECT_EQ(paths.get_application_root(), expected_root);
    EXPECT_EQ(paths.get_sessions_directory(),
              QDir(expected_root).filePath(QStringLiteral("sessions")));
}

/** @test Verifies every managed path in an explicit isolated layout. */
TEST_F(ApplicationStoragePathsTest, ResolvesCanonicalStorageLayout)
{
    const QString root = QDir(QDir::tempPath()).filePath(QStringLiteral("storage-layout-test"));
    const ApplicationStoragePaths paths(root);

    EXPECT_EQ(paths.get_settings_file(), QDir(root).filePath(QStringLiteral("settings.ini")));
    EXPECT_EQ(paths.get_sessions_directory(), QDir(root).filePath(QStringLiteral("sessions")));
    EXPECT_EQ(paths.get_session_index_file(),
              QDir(root).filePath(QStringLiteral("sessions/index.json")));
    EXPECT_EQ(paths.get_cache_directory(), QDir(root).filePath(QStringLiteral("cache")));
    EXPECT_EQ(paths.get_history_database_file(),
              QDir(root).filePath(QStringLiteral("cache/history.sqlite")));
    EXPECT_EQ(paths.get_log_index_directory(),
              QDir(root).filePath(QStringLiteral("cache/index-v2")));
    EXPECT_EQ(paths.get_diagnostics_directory(),
              QDir(root).filePath(QStringLiteral("diagnostics")));
    EXPECT_EQ(paths.get_diagnostics_log_file(),
              QDir(root).filePath(QStringLiteral("diagnostics/Qt-LogViewer.log")));
}

/** @test Verifies that the settings dialog exposes every user-relevant storage location. */
TEST_F(ApplicationStoragePathsTest, PresentsStorageLocationsInSettingsDialog)
{
    const QString marker_path =
        m_file_system.write_text_file(QStringLiteral("settings.marker"), QString());
    const QString settings_path =
        QDir(QFileInfo(marker_path).absolutePath()).filePath(QStringLiteral("settings.ini"));
    LogViewerSettings settings(settings_path, QSettings::IniFormat);
    SettingsDialog dialog(&settings);
    const ApplicationStoragePaths paths;

    const auto* categories = dialog.findChild<QListWidget*>(QStringLiteral("listWidgetCategories"));
    const auto* settings_location =
        dialog.findChild<QLineEdit*>(QStringLiteral("lineEditSettingsLocation"));
    const auto* sessions_location =
        dialog.findChild<QLineEdit*>(QStringLiteral("lineEditSessionsLocation"));
    const auto* cache_location =
        dialog.findChild<QLineEdit*>(QStringLiteral("lineEditCacheLocation"));
    const auto* diagnostics_location =
        dialog.findChild<QLineEdit*>(QStringLiteral("lineEditDiagnosticsLocation"));
    const auto* settings_location_button =
        dialog.findChild<QToolButton*>(QStringLiteral("toolButtonSettingsLocation"));

    ASSERT_NE(categories, nullptr);
    ASSERT_NE(settings_location, nullptr);
    ASSERT_NE(sessions_location, nullptr);
    ASSERT_NE(cache_location, nullptr);
    ASSERT_NE(diagnostics_location, nullptr);
    ASSERT_NE(settings_location_button, nullptr);
    EXPECT_EQ(categories->count(), 3);
    EXPECT_EQ(settings_location->text(), settings_path);
    EXPECT_EQ(sessions_location->text(), paths.get_sessions_directory());
    EXPECT_EQ(cache_location->text(), paths.get_cache_directory());
    EXPECT_EQ(diagnostics_location->text(), paths.get_diagnostics_directory());
    EXPECT_EQ(settings_location->cursorPosition(), settings_path.size());
    EXPECT_EQ(sessions_location->cursorPosition(), paths.get_sessions_directory().size());
    EXPECT_EQ(cache_location->cursorPosition(), paths.get_cache_directory().size());
    EXPECT_EQ(diagnostics_location->cursorPosition(), paths.get_diagnostics_directory().size());
    EXPECT_TRUE(settings_location_button->text().isEmpty());
    EXPECT_FALSE(settings_location_button->icon().isNull());
    EXPECT_EQ(settings_location_button->iconSize(), QSize(17, 17));
    EXPECT_FALSE(settings_location_button->icon()
                     .pixmap(settings_location_button->iconSize(), QIcon::Active)
                     .isNull());
    EXPECT_FALSE(settings_location_button->toolTip().isEmpty());
    EXPECT_GE(dialog.width(), 700);
}
