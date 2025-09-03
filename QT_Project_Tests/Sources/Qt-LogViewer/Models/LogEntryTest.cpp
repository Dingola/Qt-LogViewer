#include "Qt-LogViewer/Models/LogEntryTest.h"

#include <QDateTime>
#include <QString>

/**
 * @brief Sets up the test fixture for each test.
 *
 * This method is called before each test is run.
 */
void LogEntryTest::SetUp() {}

/**
 * @brief Tears down the test fixture after each test.
 *
 * This method is called after each test is run.
 */
void LogEntryTest::TearDown() {}

/**
 * @brief Tests the default constructor of LogEntry.
 */
TEST_F(LogEntryTest, DefaultConstructor)
{
    LogEntry entry;
    EXPECT_FALSE(entry.get_timestamp().isValid());
    EXPECT_TRUE(entry.get_level().isEmpty());
    EXPECT_TRUE(entry.get_message().isEmpty());
    EXPECT_TRUE(entry.get_app_name().isEmpty());
}

/**
 * @brief Tests the parameterized constructor of LogEntry.
 */
TEST_F(LogEntryTest, ParameterizedConstructor)
{
    QDateTime timestamp = QDateTime::currentDateTime();
    QString level = "INFO";
    QString message = "Test log message";
    QString app_name = "TestApp";
    LogFileInfo file_info("dummy.log", app_name);

    LogEntry entry(timestamp, level, message, file_info);

    EXPECT_EQ(entry.get_timestamp(), timestamp);
    EXPECT_EQ(entry.get_level(), level);
    EXPECT_EQ(entry.get_message(), message);
    EXPECT_EQ(entry.get_app_name(), app_name);
    EXPECT_EQ(entry.get_file_info().get_file_path(), "dummy.log");
}

/**
 * @brief Tests the setter and getter for timestamp.
 */
TEST_F(LogEntryTest, SetAndGetTimestamp)
{
    LogEntry entry;
    QDateTime timestamp = QDateTime::currentDateTime();
    entry.set_timestamp(timestamp);
    EXPECT_EQ(entry.get_timestamp(), timestamp);
}

/**
 * @brief Tests the setter and getter for level.
 */
TEST_F(LogEntryTest, SetAndGetLevel)
{
    LogEntry entry;
    QString level = "ERROR";
    entry.set_level(level);
    EXPECT_EQ(entry.get_level(), level);
}

/**
 * @brief Tests the setter and getter for message.
 */
TEST_F(LogEntryTest, SetAndGetMessage)
{
    LogEntry entry;
    QString message = "A log message";
    entry.set_message(message);
    EXPECT_EQ(entry.get_message(), message);
}

/**
 * @brief Tests the setter and getter for app_name.
 */
TEST_F(LogEntryTest, SetAndGetAppName)
{
    LogEntry entry;
    QString app_name = "MyApp";
    entry.set_app_name(app_name);
    EXPECT_EQ(entry.get_app_name(), app_name);
}

/**
 * @brief Tests that LogEntry can be copied correctly.
 */
TEST_F(LogEntryTest, CopyConstructorAndAssignment)
{
    QDateTime timestamp = QDateTime::currentDateTime();
    QString level = "DEBUG";
    QString message = "Copy test";
    QString app_name = "CopyApp";
    LogFileInfo file_info("dummy.log", app_name);

    LogEntry entry1(timestamp, level, message, file_info);
    LogEntry entry2(entry1);  // Copy constructor
    LogEntry entry3;
    entry3 = entry1;  // Assignment

    EXPECT_EQ(entry2.get_timestamp(), timestamp);
    EXPECT_EQ(entry2.get_level(), level);
    EXPECT_EQ(entry2.get_message(), message);
    EXPECT_EQ(entry2.get_app_name(), app_name);
    EXPECT_EQ(entry2.get_file_info().get_file_path(), "dummy.log");

    EXPECT_EQ(entry3.get_timestamp(), timestamp);
    EXPECT_EQ(entry3.get_level(), level);
    EXPECT_EQ(entry3.get_message(), message);
    EXPECT_EQ(entry3.get_app_name(), app_name);
    EXPECT_EQ(entry3.get_file_info().get_file_path(), "dummy.log");
}
