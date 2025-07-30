#include "Qt-LogViewer/Services/SettingsTest.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTemporaryFile>
#include <QTextStream>

/**
 * @brief Sets up the test fixture for each test.
 */
void SettingsTest::SetUp()
{
    QString settings_path =
        Settings::settings_file_path(QStandardPaths::AppConfigLocation, "test_settings.ini");
    QFile::remove(settings_path);  // Clean up before each test
    m_settings = new Settings(settings_path, QSettings::IniFormat);
}

/**
 * @brief Tears down the test fixture after each test.
 */
void SettingsTest::TearDown()
{
    if (m_settings != nullptr)
    {
        m_settings->clear();
        delete m_settings;
        m_settings = nullptr;
    }
}

/*
 * @brief Helper to create a temporary INI file with given content.
 */
QString SettingsTest::create_temp_ini(const QString& content)
{
    QString result;
    QTemporaryFile temp_file;
    temp_file.setAutoRemove(false);

    if (temp_file.open())
    {
        QTextStream out(&temp_file);
        out << content;
        out.flush();
        temp_file.close();
        result = temp_file.fileName();
    }

    return result;
}

/**
 * @brief Tests get_value() and set_value() for various scenarios.
 */
TEST_F(SettingsTest, GetSetValue)
{
    // Set and get value with group
    QString group = "General";
    QString key = "key2";
    QVariant value = "value2";
    m_settings->set_value(group, key, value);
    EXPECT_EQ(m_settings->get_value(group, key, QVariant()), value);

    // Get non-existing key returns default
    QString keyX = "keyX";
    QVariant default_value = "default";
    EXPECT_EQ(m_settings->get_value(group, keyX, default_value), default_value);
}

/**
 * @brief Tests child_groups() and child_keys() methods.
 */
TEST_F(SettingsTest, ChildGroupsAndKeys)
{
    // No child groups/keys
    EXPECT_TRUE(m_settings->child_groups("General").isEmpty());
    EXPECT_TRUE(m_settings->child_keys("General").isEmpty());

    // Add group and key
    QString group = "General";
    QString key = "TestKey";
    QVariant value = "TestValue";
    m_settings->set_value(group, key, value);

    EXPECT_TRUE(m_settings->child_groups("").contains(group));
    EXPECT_TRUE(m_settings->child_keys(group).contains(key));
}

/**
 * @brief Tests contains() method.
 */
TEST_F(SettingsTest, Contains)
{
    QString group = "General";
    QString key = "key1";
    QVariant value = "value1";
    m_settings->set_value(group, key, value);

    EXPECT_TRUE(m_settings->contains(group, key));
    EXPECT_FALSE(m_settings->contains(group, "not_existing"));
}

/**
 * @brief Tests clear() method.
 */
TEST_F(SettingsTest, Clear)
{
    QString group = "General";
    QString key = "key1";
    QVariant value = "value1";
    m_settings->set_value(group, key, value);
    m_settings->clear();
    EXPECT_TRUE(m_settings->child_keys(group).isEmpty());
    EXPECT_TRUE(m_settings->child_groups("").isEmpty());
}
