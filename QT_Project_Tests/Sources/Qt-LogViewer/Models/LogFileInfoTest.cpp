#include "Qt-LogViewer/Models/LogFileInfoTest.h"

/**
 * @brief Sets up the test fixture for each test.
 */
void LogFileInfoTest::SetUp() {}

/**
 * @brief Tears down the test fixture after each test.
 */
void LogFileInfoTest::TearDown() {}

/**
 * @brief Tests default construction and getters.
 */
TEST_F(LogFileInfoTest, DefaultConstruction)
{
    LogFileInfo info;
    EXPECT_EQ(info.get_file_path(), QString());
    EXPECT_EQ(info.get_app_name(), QString());
    EXPECT_EQ(info.get_file_name(), QString());
}

/**
 * @brief Tests construction with file path and app name.
 */
TEST_F(LogFileInfoTest, ConstructionWithValues)
{
    LogFileInfo info("C:/logs/log_2024_06_01.txt", "MyApp");
    EXPECT_EQ(info.get_file_path(), "C:/logs/log_2024_06_01.txt");
    EXPECT_EQ(info.get_app_name(), "MyApp");
    EXPECT_EQ(info.get_file_name(), "log_2024_06_01.txt");
}

/**
 * @brief Tests set_app_name updates the application name.
 */
TEST_F(LogFileInfoTest, SetAppNameUpdatesValue)
{
    LogFileInfo info("C:/logs/log_2024_06_01.txt");
    EXPECT_EQ(info.get_app_name(), QString());

    info.set_app_name("TestApp");
    EXPECT_EQ(info.get_app_name(), "TestApp");
}

/**
 * @brief Tests get_file_name extracts the file name from the path.
 */
TEST_F(LogFileInfoTest, GetFileNameExtractsName)
{
    LogFileInfo info("/var/log/extern_error_01.log");
    EXPECT_EQ(info.get_file_name(), "extern_error_01.log");

    LogFileInfo info2("relative/path/thirdparty_debug.log");
    EXPECT_EQ(info2.get_file_name(), "thirdparty_debug.log");

    LogFileInfo info3("just_a_file.log");
    EXPECT_EQ(info3.get_file_name(), "just_a_file.log");
}
