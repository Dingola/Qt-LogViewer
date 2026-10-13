/**
 * @file SettingsDialogTest.cpp
 * @brief Contains focused tests for the application settings dialog.
 */

#include "Qt-LogViewer/Views/App/Dialogs/SettingsDialogTest.h"

#include <QDir>
#include <QFileInfo>
#include <QPushButton>
#include <QSettings>

#include "Qt-LogViewer/Services/LogViewerSettings.h"
#include "Qt-LogViewer/Views/App/Dialogs/SettingsDialog.h"

/** @brief Creates an isolated settings service and dialog for the current test. */
void SettingsDialogTest::SetUp()
{
    const QString marker_path =
        m_file_system.write_text_file(QStringLiteral("settings-dialog.marker"), QString());
    const QString settings_path =
        QDir(QFileInfo(marker_path).absolutePath()).filePath(QStringLiteral("settings.ini"));
    m_settings = new LogViewerSettings(settings_path, QSettings::IniFormat);
    m_dialog = new SettingsDialog(m_settings);
}

/** @brief Destroys the dialog and settings service after the current test. */
void SettingsDialogTest::TearDown()
{
    delete m_dialog;
    m_dialog = nullptr;
    delete m_settings;
    m_settings = nullptr;
}

/** @test Verifies that settings actions expose one primary action and quieter alternatives. */
TEST_F(SettingsDialogTest, ExposesSemanticActionHierarchy)
{
    const auto* cancel_button =
        m_dialog->findChild<QPushButton*>(QStringLiteral("pushButtonCancel"));
    const auto* apply_button = m_dialog->findChild<QPushButton*>(QStringLiteral("pushButtonApply"));
    const auto* ok_button = m_dialog->findChild<QPushButton*>(QStringLiteral("pushButtonOk"));

    ASSERT_NE(cancel_button, nullptr);
    ASSERT_NE(apply_button, nullptr);
    ASSERT_NE(ok_button, nullptr);
    EXPECT_EQ(cancel_button->property("actionRole").toString(), QStringLiteral("ghost"));
    EXPECT_EQ(apply_button->property("actionRole").toString(), QStringLiteral("secondary"));
    EXPECT_EQ(ok_button->property("actionRole").toString(), QStringLiteral("primary"));
}
