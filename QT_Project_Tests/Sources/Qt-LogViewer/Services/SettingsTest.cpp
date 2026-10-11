#include "Qt-LogViewer/Services/SettingsTest.h"

/**
 * @brief Sets up the test fixture for each test.
 */
void SettingsTest::SetUp()
{
    const QString settings_path =
        m_file_system.write_text_file(QStringLiteral("test_settings.ini"), QString());
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
 * @brief Creates an isolated INI file with the given content.
 * @param content INI content to write.
 * @return Absolute path of the created file, or an empty string on failure.
 */
auto SettingsTest::create_temp_ini(const QString& content) -> QString
{
    const QString result =
        m_file_system.write_text_file(QStringLiteral("temporary-settings.ini"), content);
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
