#include "Qt-LogViewer/Services/LogParserTest.h"

#include <QDateTime>
#include <QTemporaryFile>
#include <QTextStream>
#include <chrono>
#include <iostream>

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
 * @test Verifies that parse_file returns an empty vector for an empty file.
 */
TEST_F(LogParserTest, ParseFileReturnsEmptyForEmptyFile)
{
    QTemporaryFile temp_file;
    ASSERT_TRUE(temp_file.open());
    temp_file.close();

    QVector<LogEntry> entries = m_parser->parse_file(temp_file.fileName());
    EXPECT_TRUE(entries.isEmpty());
}

/**
 * @test Verifies that parse_file returns an empty vector for a file with only invalid lines.
 */
TEST_F(LogParserTest, ParseFileReturnsEmptyForOnlyInvalidLines)
{
    QTemporaryFile temp_file;
    ASSERT_TRUE(temp_file.open());
    QTextStream out(&temp_file);
    out << "invalid line 1\n";
    out << "invalid line 2\n";
    out.flush();
    temp_file.close();

    QVector<LogEntry> entries = m_parser->parse_file(temp_file.fileName());
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

/**
 * @test Verifies that parse_line works when the format string does not include a timestamp.
 */
TEST_F(LogParserTest, ParseLineWithoutTimestampField)
{
    // Format: level message app_name
    QString format = "{level} {message} {app_name}";
    LogParser parser(format);

    QString line = "Info HelloWorld MyApp";
    LogEntry entry = parser.parse_line(line, "dummy.log");

    EXPECT_TRUE(entry.get_timestamp().isNull() || !entry.get_timestamp().isValid());
    EXPECT_EQ(entry.get_level(), "Info");
    EXPECT_EQ(entry.get_message(), "HelloWorld");
    EXPECT_EQ(entry.get_app_name(), "MyApp");
}

/**
 * @test Verifies that get_field_order returns the correct field order from the format string.
 */
TEST_F(LogParserTest, GetFieldOrderReturnsCorrectOrder)
{
    // The default format string used in SetUp:
    // "{timestamp} {level} {message} {app_name} [{file}:{line} ({function})]"
    LogFieldOrder order = m_parser->get_field_order();

    QVector<QString> expected_fields = {"timestamp", "level", "message", "app_name",
                                        "file",      "line",  "function"};

    EXPECT_EQ(order.fields, expected_fields);
}

/**
 * @test Verifies that LogParser handles a format string with no placeholders.
 */
TEST_F(LogParserTest, LogParserWithNoPlaceholders)
{
    QString format = "static text only";
    LogParser parser(format);

    QRegularExpression regex = parser.get_pattern();
    LogFieldOrder order = parser.get_field_order();

    // The regex should match only the exact static text
    QRegularExpressionMatch match = regex.match("static text only");
    EXPECT_TRUE(match.hasMatch());

    // There should be no fields in the order
    EXPECT_TRUE(order.fields.isEmpty());

    // Should not match anything else
    QRegularExpressionMatch mismatch = regex.match("other text");
    EXPECT_FALSE(mismatch.hasMatch());
}

/**
 * @test Measures baseline runtime of parse_line for a stable valid log line.
 */
TEST_F(LogParserTest, ParseLinePerformanceBaseline)
{
    const QString line =
        "2024-01-01 12:34:56 Debug This is a debug message MyApp [file.cpp:42 (func())]";
    const QString file_path = "dummy.log";
    constexpr int iterations = 200000;

    volatile int parsed_count = 0;

    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < iterations; ++i)
    {
        LogEntry entry = m_parser->parse_line(line, file_path);
        if (!entry.get_level().isEmpty())
        {
            ++parsed_count;
        }
    }
    auto end = std::chrono::steady_clock::now();

    const auto elapsed_us =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    const double avg_us_per_line =
        static_cast<double>(elapsed_us) / static_cast<double>(iterations);

    EXPECT_EQ(parsed_count, iterations);

    std::cout << "[PERF] LogParser::parse_line baseline: " << elapsed_us << " us total for "
              << iterations << " iterations (" << avg_us_per_line << " us/line)" << std::endl;
}

/**
 * @test Measures baseline runtime of parse_file for a representative synthetic file.
 */
TEST_F(LogParserTest, ParseFilePerformanceBaseline)
{
    constexpr int lines_per_file = 10000;
    constexpr int runs = 5;

    QTemporaryFile temp_file;
    ASSERT_TRUE(temp_file.open());

    QTextStream out(&temp_file);
    for (int i = 0; i < lines_per_file; ++i)
    {
        out << "2024-01-01 12:34:56 Info Message " << i << " MyApp [file.cpp:42 (func())]\n";
    }
    out.flush();
    temp_file.close();

    // Warmup run to reduce first-run variance in the measured runs.
    QVector<LogEntry> warmup_entries = m_parser->parse_file(temp_file.fileName());
    ASSERT_EQ(warmup_entries.size(), lines_per_file);

    qint64 total_elapsed_ms = 0;
    int total_entries = 0;

    for (int run = 0; run < runs; ++run)
    {
        auto start = std::chrono::steady_clock::now();
        QVector<LogEntry> entries = m_parser->parse_file(temp_file.fileName());
        auto end = std::chrono::steady_clock::now();

        EXPECT_EQ(entries.size(), lines_per_file);

        total_entries += entries.size();
        total_elapsed_ms +=
            std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    }

    const double avg_ms_per_run = static_cast<double>(total_elapsed_ms) / static_cast<double>(runs);
    const double entries_per_second =
        (static_cast<double>(total_entries) * 1000.0) / static_cast<double>(total_elapsed_ms);

    std::cout << "[PERF] LogParser::parse_file baseline: " << total_elapsed_ms << " ms total for "
              << runs << " runs x " << lines_per_file << " lines"
              << " (avg " << avg_ms_per_run << " ms/run, " << entries_per_second << " lines/s)"
              << std::endl;
}
