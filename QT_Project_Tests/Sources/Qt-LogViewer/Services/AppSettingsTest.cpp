#include "Qt-LogViewer/Services/AppSettingsTest.h"

#include <QCoreApplication>
#include <QFile>
#include <QString>

/**
 * @brief Sets up the test fixture for each test.
 */
void AppSettingsTest::SetUp()
{
    m_app_settings = new AppSettings();
}

/**
 * @brief Tears down the test fixture after each test.
 */
void AppSettingsTest::TearDown()
{
    if (m_app_settings != nullptr)
    {
        m_app_settings->clear();
        delete m_app_settings;
        m_app_settings = nullptr;
    }
}

/**
 * @brief Tests get_theme() and set_theme() methods.
 */
TEST_F(AppSettingsTest, Theme)
{
    // Default theme
    EXPECT_EQ(m_app_settings->get_theme(), QString("Dark"));

    // Set theme to "Light"
    m_app_settings->set_theme("Light");
    EXPECT_EQ(m_app_settings->get_theme(), QString("Light"));

    // Set theme to "Dark"
    m_app_settings->set_theme("Dark");
    EXPECT_EQ(m_app_settings->get_theme(), QString("Dark"));
}

/**
 * @brief Tests get_language() and set_language() methods.
 */
TEST_F(AppSettingsTest, Language)
{
    // Default language
    EXPECT_EQ(m_app_settings->get_language(), QString("en"));

    // Set language to "de"
    m_app_settings->set_language("de");
    EXPECT_EQ(m_app_settings->get_language(), QString("de"));

    // Set language to "en"
    m_app_settings->set_language("en");
    EXPECT_EQ(m_app_settings->get_language(), QString("en"));
}

/**
 * @brief Tests persistence of theme and language settings.
 */
TEST_F(AppSettingsTest, Persistence)
{
    QString file_path = QCoreApplication::applicationDirPath() + "/appsettings_test.ini";

    // Set values
    m_app_settings->set_theme("Light");
    m_app_settings->set_language("de");
    m_app_settings->save_to_file(file_path);

    // Load into a new instance
    AppSettings loaded_settings;
    loaded_settings.load_from_file(file_path);

    EXPECT_EQ(loaded_settings.get_theme(), QString("Light"));
    EXPECT_EQ(loaded_settings.get_language(), QString("de"));

    QFile::remove(file_path);
}
