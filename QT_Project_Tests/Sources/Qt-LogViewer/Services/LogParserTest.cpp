#include "Qt-LogViewer/Services/LogParserTest.h"

#include <QDateTime>
#include <QTemporaryFile>
#include <QTextStream>

/**
 * @brief Sets up the test fixture for each test.
 *
 * Initializes the LogParser instance with a default format string.
 */
void LogParserTest::SetUp()
{
    // Default format: timestamp level message app_name [file:line (function)]
    QString format = "{timestamp} {level} {message} {app_name} [{file}:{line} ({function})]";
    m_parser = new LogParser(format);
}

/**
 * @brief Tears down the test fixture after each test.
 *
 * Deletes the LogParser instance and resets the pointer.
 */
void LogParserTest::TearDown()
{
    delete m_parser;
    m_parser = nullptr;
}

/**
 * @test Verifies that the parser generates the correct regex pattern from the format string.
 */
TEST_F(LogParserTest, PatternIsGeneratedCorrectly)
{
    QString log_line =
        "2024-01-01 12:34:56 Debug This is a debug message MyApp [file.cpp:42 (func())]";
    QRegularExpression re = m_parser->get_pattern();
    QRegularExpressionMatch match = re.match(log_line);
    ASSERT_TRUE(match.hasMatch());
    EXPECT_EQ(match.captured(1), "2024-01-01 12:34:56");
    EXPECT_EQ(match.captured(2), "Debug");
    EXPECT_EQ(match.captured(3), "This is a debug message");
    EXPECT_EQ(match.captured(4), "MyApp");
    EXPECT_EQ(match.captured(5), "file.cpp");
    EXPECT_EQ(match.captured(6), "42");
    EXPECT_EQ(match.captured(7), "func()");
}

/**
 * @test Verifies that a valid log line is parsed correctly into a LogEntry.
 */
TEST_F(LogParserTest, ParseValidLogLine)
{
    QString line = "2024-01-01 12:34:56 Debug This is a debug message MyApp [file.cpp:42 (func())]";
    LogEntry entry = m_parser->parse_line(line, "dummy.log");

    EXPECT_EQ(entry.get_timestamp(),
              QDateTime::fromString("2024-01-01 12:34:56", "yyyy-MM-dd HH:mm:ss"));
    EXPECT_EQ(entry.get_level(), "Debug");
    EXPECT_EQ(entry.get_message(), "This is a debug message");
    EXPECT_EQ(entry.get_app_name(), "MyApp");
}

/**
 * @test Verifies that an invalid log line returns a default LogEntry.
 */
TEST_F(LogParserTest, ParseInvalidLogLineReturnsDefault)
{
    QString line = "invalid log line";
    LogEntry entry = m_parser->parse_line(line, "dummy.log");

    EXPECT_FALSE(entry.get_timestamp().isValid());
    EXPECT_TRUE(entry.get_level().isEmpty());
    EXPECT_TRUE(entry.get_app_name().isEmpty());
    EXPECT_TRUE(entry.get_message().isEmpty());
}

/**
 * @test Verifies that parse_file parses all valid lines and ignores invalid ones.
 */
TEST_F(LogParserTest, ParseFileParsesAllValidLines)
{
    QTemporaryFile temp_file;
    ASSERT_TRUE(temp_file.open());
    QTextStream out(&temp_file);
    out << "2024-01-01 12:34:56 Info Info message App1 [file1.cpp:10 (main())]\n";
    out << "invalid line\n";
    out << "2024-01-01 12:35:00 Error Error message App2 [file2.cpp:20 (foo())]\n";
    out.flush();
    temp_file.close();

    QVector<LogEntry> entries = m_parser->parse_file(temp_file.fileName());

    ASSERT_EQ(entries.size(), 2);
    EXPECT_EQ(entries[0].get_timestamp(),
              QDateTime::fromString("2024-01-01 12:34:56", "yyyy-MM-dd HH:mm:ss"));
    EXPECT_EQ(entries[0].get_level(), "Info");
    EXPECT_EQ(entries[0].get_message(), "Info message");
    EXPECT_EQ(entries[0].get_app_name(), "App1");
    EXPECT_EQ(entries[1].get_timestamp(),
              QDateTime::fromString("2024-01-01 12:35:00", "yyyy-MM-dd HH:mm:ss"));
    EXPECT_EQ(entries[1].get_level(), "Error");
    EXPECT_EQ(entries[1].get_message(), "Error message");
    EXPECT_EQ(entries[1].get_app_name(), "App2");
}

/**
 * @test Verifies that parse_file returns an empty vector if the file does not exist.
 */
TEST_F(LogParserTest, ParseFileReturnsEmptyIfFileNotFound)
{
    QVector<LogEntry> entries = m_parser->parse_file("nonexistent_file.log");
    EXPECT_TRUE(entries.isEmpty());
}

/**
 * @test Verifies parsing with a different field order in the format string.
 */
TEST_F(LogParserTest, ParseLineWithDifferentFieldOrder)
{
    // Format: timestamp app_name level message
    QString format = "{timestamp} {app_name} {level} {message}";
    LogParser parser(format);

    QString line = "2024-01-01 12:34:56 MyApp Debug This is a debug message";
    LogEntry entry = parser.parse_line(line, "dummy.log");

    EXPECT_EQ(entry.get_timestamp(),
              QDateTime::fromString("2024-01-01 12:34:56", "yyyy-MM-dd HH:mm:ss"));
    EXPECT_EQ(entry.get_level(), "Debug");
    EXPECT_EQ(entry.get_message(), "This is a debug message");
    EXPECT_EQ(entry.get_app_name(), "MyApp");
}

/**
 * @test Verifies parsing when some fields are missing in the format string.
 */
TEST_F(LogParserTest, ParseLineWithMissingFields)
{
    // Format: timestamp level message
    QString format = "{timestamp} {level} {message}";
    LogParser parser(format);

    QString line = "2024-01-01 12:34:56 Debug This is a debug message";
    LogEntry entry = parser.parse_line(line, "dummy.log");

    EXPECT_EQ(entry.get_timestamp(),
              QDateTime::fromString("2024-01-01 12:34:56", "yyyy-MM-dd HH:mm:ss"));
    EXPECT_EQ(entry.get_level(), "Debug");
    EXPECT_EQ(entry.get_message(), "This is a debug message");
    EXPECT_TRUE(entry.get_app_name().isEmpty());
}
