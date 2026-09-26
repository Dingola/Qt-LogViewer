/**
 * @file LogColumnSchemaBuilderTest.cpp
 * @brief Tests deterministic log-column schema construction.
 */

#include "Qt-LogViewer/Models/LogColumnSchemaBuilderTest.h"

#include "Qt-LogViewer/Models/LogColumnSchemaBuilder.h"

/**
 * @brief Verifies that one profile preserves its resolved parser field order.
 */
TEST_F(LogColumnSchemaBuilderTest, BuildsColumnsInProfileOrder)
{
    const LogParsingProfile profile = LogParsingProfile::create_default(
        QStringLiteral("{level}|{message}|{request_id}"), QStringLiteral("Pipe"));

    const QVector<LogFieldDefinition> columns = LogColumnSchemaBuilder::build(profile);

    ASSERT_EQ(columns.size(), 3);
    EXPECT_EQ(columns.at(0).id, LogField::Level);
    EXPECT_EQ(columns.at(1).id, LogField::Message);
    EXPECT_EQ(columns.at(2).id, QStringLiteral("request_id"));
    EXPECT_EQ(columns.at(2).display_name, QStringLiteral("Request id"));
}

/**
 * @brief Verifies that several profiles contribute each field identifier exactly once.
 */
TEST_F(LogColumnSchemaBuilderTest, MergesProfilesWithoutDuplicateFields)
{
    const LogParsingProfile first = LogParsingProfile::create_default(
        QStringLiteral("{timestamp}|{message}"), QStringLiteral("First"));
    const LogParsingProfile second = LogParsingProfile::create_default(
        QStringLiteral("{level}|{message}|{trace_id}"), QStringLiteral("Second"));

    const QVector<LogFieldDefinition> columns =
        LogColumnSchemaBuilder::build(QVector<LogParsingProfile>{first, second});

    ASSERT_EQ(columns.size(), 4);
    EXPECT_EQ(columns.at(0).id, LogField::Timestamp);
    EXPECT_EQ(columns.at(1).id, LogField::Message);
    EXPECT_EQ(columns.at(2).id, LogField::Level);
    EXPECT_EQ(columns.at(3).id, QStringLiteral("trace_id"));
}
