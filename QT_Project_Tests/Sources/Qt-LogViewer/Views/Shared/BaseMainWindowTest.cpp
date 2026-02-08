#include "Qt-LogViewer/Views/Shared/BaseMainWindowTest.h"

#include <QByteArray>
#include <QFile>
#include <QMainWindow>
#include <QTest>
#include <QWidget>

#include "QtWidgetsCommonLib/Utils/StylesheetLoader.h"

/**
 * @brief Creates a temporary QSS file with the given content.
 * @param content The QSS content to write to the file.
 * @return The path to the temporary QSS file.
 */
QString BaseMainWindowTest::create_temp_qss(const QString& content)
{
    QTemporaryFile temp_file;
    temp_file.setAutoRemove(false);
    QString result;

    if (temp_file.open())
    {
        temp_file.write(content.toUtf8());
        temp_file.close();
        result = temp_file.fileName();
    }

    return result;
}

/**
 * @brief Sets up the test fixture before each test.
 */
void BaseMainWindowTest::SetUp()
{
    m_mock_prefs = new MockPreferences();
    m_window = new TestableBaseMainWindow(m_mock_prefs);
    m_temp_qss_path.clear();
}

/**
 * @brief Tears down the test fixture after each test.
 */
void BaseMainWindowTest::TearDown()
{
    delete m_window;
    m_window = nullptr;
    delete m_mock_prefs;
    m_mock_prefs = nullptr;

    if (!m_temp_qss_path.isEmpty())
    {
        QFile::remove(m_temp_qss_path);
    }

    m_temp_qss_path.clear();
}

/**
 * @brief Tests the construction of the BaseMainWindow and its stylesheet loader.
 */
TEST_F(BaseMainWindowTest, ConstructionAndStylesheetLoader)
{
    EXPECT_NE(m_window, nullptr);
    EXPECT_NE(m_window->get_stylesheet_loader(), nullptr);
}

/**
 * @brief Tests that saving window settings calls the preferences methods.
 */
TEST_F(BaseMainWindowTest, SaveWindowSettingsCallsPreferences)
{
    EXPECT_CALL(*m_mock_prefs, set_mainwindow_geometry(testing::_)).Times(1);
    EXPECT_CALL(*m_mock_prefs, set_mainwindow_state(testing::_)).Times(1);
    EXPECT_CALL(*m_mock_prefs, set_mainwindow_windowstate(testing::_)).Times(1);

    m_window->save_window_settings();
}

/**
 * @brief Tests that restoring window settings calls the preferences methods.
 */
TEST_F(BaseMainWindowTest, RestoreWindowSettingsCallsPreferences)
{
    EXPECT_CALL(*m_mock_prefs, get_mainwindow_geometry()).Times(1);
    EXPECT_CALL(*m_mock_prefs, get_mainwindow_state()).Times(1);
    EXPECT_CALL(*m_mock_prefs, get_mainwindow_windowstate()).Times(1);

    m_window->restore_window_settings();
}

/**
 * @brief Tests that the main window can be shown and closed without exceptions.
 */
TEST_F(BaseMainWindowTest, ShowAndCloseWindow)
{
    EXPECT_NO_THROW(m_window->show());
    EXPECT_NO_THROW(m_window->close());
}

/**
 * @brief Tests that emitting the themeChanged signal on preferences
 *        causes BaseMainWindow to load and apply the correct stylesheet
 *        for the given theme, including variable substitution.
 */
TEST_F(BaseMainWindowTest, ThemeChangeSignalLoadsStylesheet)
{
    auto* loader = m_window->get_stylesheet_loader();
    ASSERT_NE(loader, nullptr);

    QString qss = R"(
@Variables[Name="Light"] {
    @PrimaryColor: #abcdef;
}
QWidget { background: @PrimaryColor; }
)";
    m_temp_qss_path = create_temp_qss(qss);
    ASSERT_FALSE(m_temp_qss_path.isEmpty());

    EXPECT_TRUE(loader->load_stylesheet(m_temp_qss_path, "Light"));

    emit m_mock_prefs->themeChanged("Light");

    EXPECT_EQ(loader->get_current_theme_name(), "Light");
    EXPECT_TRUE(loader->get_current_stylesheet().contains("#abcdef"));
    EXPECT_FALSE(loader->get_current_stylesheet().contains("@PrimaryColor"));
}
