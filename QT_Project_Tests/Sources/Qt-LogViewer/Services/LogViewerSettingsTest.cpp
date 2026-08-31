#include "Qt-LogViewer/Services/LogViewerSettingsTest.h"

#include <QString>

/**
 * @brief Sets up the test fixture for each test.
 */
void LogViewerSettingsTest::SetUp()
{
    m_app_settings = new LogViewerSettings();
}

/**
 * @brief Tears down the test fixture after each test.
 */
void LogViewerSettingsTest::TearDown()
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
TEST_F(LogViewerSettingsTest, Theme)
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
TEST_F(LogViewerSettingsTest, Language)
{
    // Default language
    EXPECT_EQ(m_app_settings->get_language_code(), QString("en"));

    // Set language to "de"
    m_app_settings->set_language_code("de");
    EXPECT_EQ(m_app_settings->get_language_code(), QString("de"));

    // Set language to "en"
    m_app_settings->set_language_code("en");
    EXPECT_EQ(m_app_settings->get_language_code(), QString("en"));
}

/**
 * @brief Tests get_language_name() and set_language_name() methods.
 */
TEST_F(LogViewerSettingsTest, LanguageName)
{
    // Default language name
    EXPECT_EQ(m_app_settings->get_language_name(), QString("English"));

    // Set language name to "Deutsch"
    m_app_settings->set_language_name("Deutsch");
    EXPECT_EQ(m_app_settings->get_language_name(), QString("Deutsch"));

    // Set language name to "English"
    m_app_settings->set_language_name("English");
    EXPECT_EQ(m_app_settings->get_language_name(), QString("English"));
}

/**
 * @brief Tests get_mainwindow_geometry() and set_mainwindow_geometry() methods.
 */
TEST_F(LogViewerSettingsTest, MainWindowGeometry)
{
    // Default geometry should be empty
    EXPECT_TRUE(m_app_settings->get_mainwindow_geometry().isEmpty());

    // Set geometry
    QByteArray geometry = QByteArray::fromHex("01020304");
    m_app_settings->set_mainwindow_geometry(geometry);
    EXPECT_EQ(m_app_settings->get_mainwindow_geometry(), geometry);

    // Set geometry to empty
    m_app_settings->set_mainwindow_geometry(QByteArray());
    EXPECT_TRUE(m_app_settings->get_mainwindow_geometry().isEmpty());
}

/**
 * @brief Tests get_mainwindow_state() and set_mainwindow_state() methods.
 */
TEST_F(LogViewerSettingsTest, MainWindowState)
{
    // Default state should be empty
    EXPECT_TRUE(m_app_settings->get_mainwindow_state().isEmpty());

    // Set state
    QByteArray state = QByteArray::fromHex("0A0B0C0D");
    m_app_settings->set_mainwindow_state(state);
    EXPECT_EQ(m_app_settings->get_mainwindow_state(), state);

    // Set state to empty
    m_app_settings->set_mainwindow_state(QByteArray());
    EXPECT_TRUE(m_app_settings->get_mainwindow_state().isEmpty());
}

/**
 * @brief Tests get_mainwindow_windowstate() and set_mainwindow_windowstate() methods.
 */
TEST_F(LogViewerSettingsTest, MainWindowWindowState)
{
    // Default window state should be Qt::WindowNoState (0)
    EXPECT_EQ(m_app_settings->get_mainwindow_windowstate(), 0);

    // Set window state to minimized (1)
    m_app_settings->set_mainwindow_windowstate(1);
    EXPECT_EQ(m_app_settings->get_mainwindow_windowstate(), 1);

    // Set window state to maximized (2)
    m_app_settings->set_mainwindow_windowstate(2);
    EXPECT_EQ(m_app_settings->get_mainwindow_windowstate(), 2);

    // Set window state back to normal (0)
    m_app_settings->set_mainwindow_windowstate(0);
    EXPECT_EQ(m_app_settings->get_mainwindow_windowstate(), 0);
}

/**
 * @brief Tests that parsing profiles preserve identity, names and parser configurations.
 */
TEST_F(LogViewerSettingsTest, LogParsingProfiles)
{
    const LogParsingProfile first = LogParsingProfile::create_default(
        QStringLiteral("{timestamp} [{level}] {message}"), QStringLiteral("Application log"));
    const LogParsingProfile second = LogParsingProfile::create_default(
        QStringLiteral("{level}|{message}|{request_id}"), QStringLiteral("Pipe log"));

    ASSERT_TRUE(m_app_settings->set_log_parsing_profiles({first, second}));

    QString error;
    const QVector<LogParsingProfile> loaded = m_app_settings->get_log_parsing_profiles(&error);

    ASSERT_TRUE(error.isEmpty()) << qPrintable(error);
    ASSERT_EQ(loaded.size(), 2);

    EXPECT_EQ(loaded.at(0).get_id(), first.get_id());
    EXPECT_EQ(loaded.at(0).get_name(), first.get_name());
    EXPECT_EQ(loaded.at(0).get_configuration().to_json(), first.get_configuration().to_json());

    EXPECT_EQ(loaded.at(1).get_id(), second.get_id());
    EXPECT_EQ(loaded.at(1).get_name(), second.get_name());
    EXPECT_EQ(loaded.at(1).get_configuration().to_json(), second.get_configuration().to_json());
}

/**
 * @brief Tests that an empty profile collection replaces previously stored profiles.
 */
TEST_F(LogViewerSettingsTest, EmptyLogParsingProfiles)
{
    const LogParsingProfile profile =
        LogParsingProfile::create_default(QStringLiteral("{message}"));

    ASSERT_TRUE(m_app_settings->set_log_parsing_profiles({profile}));
    ASSERT_TRUE(m_app_settings->set_log_parsing_profiles({}));

    QString error;
    const QVector<LogParsingProfile> loaded = m_app_settings->get_log_parsing_profiles(&error);

    EXPECT_TRUE(error.isEmpty());
    EXPECT_TRUE(loaded.isEmpty());
}

/**
 * @brief Tests that malformed parsing-profile settings are reported without partial results.
 */
TEST_F(LogViewerSettingsTest, InvalidLogParsingProfiles)
{
    m_app_settings->set_value(QStringLiteral("ParsingProfiles"), QStringLiteral("profiles"),
                              QByteArray("{invalid"));

    QString error;
    const QVector<LogParsingProfile> loaded = m_app_settings->get_log_parsing_profiles(&error);

    EXPECT_TRUE(loaded.isEmpty());
    EXPECT_FALSE(error.isEmpty());
}
