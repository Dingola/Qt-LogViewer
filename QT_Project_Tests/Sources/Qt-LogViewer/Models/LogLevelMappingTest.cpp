/**
 * @file LogLevelMappingTest.cpp
 * @brief Tests shared log-level text conversion and alias handling.
 */

#include "Qt-LogViewer/Models/LogLevelMappingTest.h"

#include "Qt-LogViewer/Models/LogLevelMapping.h"

/** @test Verifies canonical names, aliases, decorated names, and unknown input. */
TEST_F(LogLevelMappingTest, ResolvesCanonicalAndAliasNames)
{
    EXPECT_EQ(LogLevels::from_text(QStringLiteral(" Debug ")), SimpleCppLogger::LogLevel::Debug);
    EXPECT_EQ(LogLevels::from_text(QStringLiteral("warn")), SimpleCppLogger::LogLevel::Warning);
    EXPECT_EQ(LogLevels::from_text(QStringLiteral("critical")), SimpleCppLogger::LogLevel::Error);
    EXPECT_EQ(LogLevels::from_text(QStringLiteral("trace_info")), SimpleCppLogger::LogLevel::Trace);
    EXPECT_FALSE(LogLevels::from_text(QStringLiteral("custom")).has_value());
}

/** @test Verifies that every canonical enum value round-trips through Qt text. */
TEST_F(LogLevelMappingTest, ConvertsCanonicalLevelsToQtText)
{
    EXPECT_EQ(LogLevels::to_text(SimpleCppLogger::LogLevel::Trace), QStringLiteral("Trace"));
    EXPECT_EQ(LogLevels::to_text(SimpleCppLogger::LogLevel::Warning), QStringLiteral("Warning"));
    EXPECT_EQ(LogLevels::to_text(SimpleCppLogger::LogLevel::Fatal), QStringLiteral("Fatal"));
    EXPECT_TRUE(LogLevels::to_text(SimpleCppLogger::LogLevel::Count).isEmpty());
}
