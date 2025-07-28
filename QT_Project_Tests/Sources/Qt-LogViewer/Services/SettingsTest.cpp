#include "Qt-LogViewer/Services/SettingsTest.h"

#include <QCoreApplication>
#include <QFile>
#include <QDir>
#include <QTemporaryFile>
#include <QTextStream>

/**
 * @brief Sets up the test fixture for each test.
 */
void SettingsTest::SetUp()
{
#ifdef Q_OS_MAC
    QString settings_path = QDir::homePath() + "/Library/Preferences/com.adrianhelbig_tests.Qt-LogViewer_Tests.plist";
    QFile::remove(settings_path);
#endif
    m_settings = new Settings();
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
 * @brief Tests get_value() for various scenarios.
 */
TEST_F(SettingsTest, GetValue)
{
    // Key exists
    QString key = "key1";
    QVariant value = "default value";
    m_settings->set_value(key, value);
    EXPECT_EQ(m_settings->get_value(key), value);

    // Key exists in group
    QString group = "General";
    key = "key2";
    m_settings->set_value(group, key, value);
    EXPECT_EQ(m_settings->get_value(group, key, QVariant()), value);

    // Key does not exist
    key = "key3";
    EXPECT_EQ(m_settings->get_value(key), QVariant());

    // Key does not exist in group
    key = "key4";
    EXPECT_EQ(m_settings->get_value(group, key, QVariant()), QVariant());

    // Key does not exist, default value provided
    key = "key5";
    QVariant default_value = "test default value";
    EXPECT_EQ(m_settings->get_value(key, default_value), default_value);

    // Key does not exist in group, default value provided
    key = "key6";
    EXPECT_EQ(m_settings->get_value(group, key, default_value), default_value);
}

/**
 * @brief Tests set_value() for various scenarios.
 */
TEST_F(SettingsTest, SetValue)
{
    // Key does not exist
    QString key = "new key";
    QVariant value = "new value";
    m_settings->set_value(key, value);
    EXPECT_TRUE(m_settings->all_keys().contains(key));

    // Key does not exist in group
    QString group = "General";
    QString key_2 = "new key 2";
    value = "new value 2";
    m_settings->set_value(group, key_2, value);
    EXPECT_TRUE(m_settings->child_keys(group).contains(key_2));

    // Same key exists
    value = "updated value";
    m_settings->set_value(key, value);
    EXPECT_EQ(m_settings->get_value(key), value);

    // Same key exists in group
    value = "updated value 2";
    m_settings->set_value(group, key_2, value);
    EXPECT_EQ(m_settings->get_value(group, key_2, QVariant()), value);
}

/**
 * @brief Tests child_groups() method.
 */
TEST_F(SettingsTest, ChildGroups)
{
    // No child groups
    EXPECT_TRUE(m_settings->child_groups().isEmpty());

    // Add child group
    QString group = "TestGroup";
    QString key = "new key";
    QVariant value = "new value";
    m_settings->set_value(group, key, value);
    EXPECT_TRUE(m_settings->child_groups().contains(group));
}

/**
 * @brief Tests child_keys() method.
 */
TEST_F(SettingsTest, ChildKeys)
{
    QString group = "General";
    // No child keys
    EXPECT_TRUE(m_settings->child_keys(group).isEmpty());

    // Add child key
    QString key = "TestKey";
    QVariant value = "TestValue";
    m_settings->set_value(group, key, value);
    EXPECT_TRUE(m_settings->child_keys(group).contains(key));
}

/**
 * @brief Tests all_keys() method.
 */
TEST_F(SettingsTest, AllKeys)
{
    // No keys
    EXPECT_TRUE(m_settings->all_keys().isEmpty());

    // Add keys
    QString key1 = "key1";
    QVariant value1 = "value1";
    m_settings->set_value(key1, value1);

    QString key2 = "key2";
    QVariant value2 = "value2";
    m_settings->set_value(key2, value2);

    QStringList keys = m_settings->all_keys();
    EXPECT_TRUE(keys.contains(key1));
    EXPECT_TRUE(keys.contains(key2));
}

/**
 * @brief Tests contains() method.
 */
TEST_F(SettingsTest, Contains)
{
    // Key exists
    QString key = "key1";
    QVariant value = "default value";
    m_settings->set_value(key, value);
    EXPECT_TRUE(m_settings->contains(key));

    // Key and group exist
    QString group = "General";
    key = "key2";
    value = "default value 2";
    m_settings->set_value(group, key, value);
    EXPECT_TRUE(m_settings->contains(group, key));

    // Key does not exist
    key = "key3";
    EXPECT_FALSE(m_settings->contains(group, key));
}

/**
 * @brief Tests clear() method.
 */
TEST_F(SettingsTest, Clear)
{
    QString key = "key1";
    QVariant value = "default value";
    m_settings->set_value(key, value);
    m_settings->clear();
    EXPECT_TRUE(m_settings->child_keys("General").isEmpty());
    EXPECT_TRUE(m_settings->all_keys().isEmpty());
}

/**
 * @brief Tests load_from_file() method.
 */
TEST_F(SettingsTest, LoadFromFile)
{
    QString ini_content =
        "[General]\n"
        "key1=value1\n"
        "key2=value2\n"
        "[%General]\n"
        "key3=value3\n"
        "[TestGroup]\n"
        "Group/key4=value4\n"
        "[TestGroup_2]\n"
        "Group_2/Group/key5=value5\n";
    QString file_path = create_temp_ini(ini_content);

    m_settings->load_from_file(file_path);

    EXPECT_EQ(m_settings->get_value("key1"), QVariant("value1"));
    EXPECT_EQ(m_settings->get_value("key2"), QVariant("value2"));
    EXPECT_EQ(m_settings->get_value("General", "key3", QVariant()), QVariant("value3"));
    EXPECT_EQ(m_settings->get_value("TestGroup", "Group/key4", QVariant()), QVariant("value4"));
    EXPECT_EQ(m_settings->get_value("TestGroup_2", "Group_2/Group/key5", QVariant()),
              QVariant("value5"));

    QFile::remove(file_path);
}

/**
 * @brief Tests save_to_file() method.
 */
TEST_F(SettingsTest, SaveToFile)
{
    QString file_path =
        QCoreApplication::applicationDirPath() + "/appsettingstest_save_settings.ini";
    m_settings->set_value("key1", "value1");
    m_settings->set_value("key2", "value2");
    m_settings->set_value("General", "key3", "value3");
    m_settings->set_value("TestGroup", "Group/key4", "value4");
    m_settings->set_value("TestGroup_2", "Group_2/Group/key5", "value5");

    m_settings->save_to_file(file_path);

    Settings loaded_settings;
    loaded_settings.load_from_file(file_path);

    EXPECT_EQ(loaded_settings.get_value("key1"), QVariant("value1"));
    EXPECT_EQ(loaded_settings.get_value("key2"), QVariant("value2"));
    EXPECT_EQ(loaded_settings.get_value("General", "key3", QVariant()), QVariant("value3"));
    EXPECT_EQ(loaded_settings.get_value("TestGroup", "Group/key4", QVariant()), QVariant("value4"));
    EXPECT_EQ(loaded_settings.get_value("TestGroup_2", "Group_2/Group/key5", QVariant()),
              QVariant("value5"));

    QFile::remove(file_path);
}
