#include "Qt-LogViewer/Models/LogModelTest.h"

#include <QDateTime>

/**
 * @brief Sets up the test fixture for each test.
 */
void LogModelTest::SetUp()
{
    m_model.clear();
}

/**
 * @brief Tears down the test fixture after each test.
 */
void LogModelTest::TearDown()
{
    m_model.clear();
}

/**
 * @brief Tests that the model is initially empty.
 */
TEST_F(LogModelTest, ModelIsInitiallyEmpty)
{
    EXPECT_EQ(m_model.rowCount(), 0);
    EXPECT_EQ(m_model.columnCount(), LogModel::ColumnCount);
    EXPECT_TRUE(m_model.get_entries().isEmpty());
}

/**
 * @brief Tests adding a log entry to the model.
 */
TEST_F(LogModelTest, AddEntryIncreasesRowCount)
{
    LogEntry entry(QDateTime::currentDateTime(), "INFO", "Test message",
                   LogFileInfo("dummy.log", "TestApp"));
    m_model.add_entry(entry);

    EXPECT_EQ(m_model.rowCount(), 1);
    EXPECT_EQ(m_model.get_entry(0).get_message(), "Test message");
}

/**
 * @brief Tests clearing the model removes all entries.
 */
TEST_F(LogModelTest, ClearRemovesAllEntries)
{
    m_model.add_entry(
        LogEntry(QDateTime::currentDateTime(), "INFO", "A", LogFileInfo("file1.log", "App")));
    m_model.add_entry(
        LogEntry(QDateTime::currentDateTime(), "ERROR", "B", LogFileInfo("file2.log", "App")));
    EXPECT_EQ(m_model.rowCount(), 2);

    m_model.clear();
    EXPECT_EQ(m_model.rowCount(), 0);
    EXPECT_TRUE(m_model.get_entries().isEmpty());
}

/**
 * @brief Tests set_entries replaces all entries in the model.
 */
TEST_F(LogModelTest, SetEntriesReplacesAllEntries)
{
    QVector<LogEntry> entries;
    entries.append(
        LogEntry(QDateTime::currentDateTime(), "INFO", "A", LogFileInfo("file1.log", "App1")));
    entries.append(
        LogEntry(QDateTime::currentDateTime(), "ERROR", "B", LogFileInfo("file2.log", "App2")));

    m_model.set_entries(entries);

    EXPECT_EQ(m_model.rowCount(), 2);
    EXPECT_EQ(m_model.get_entry(1).get_level(), "ERROR");
    EXPECT_EQ(m_model.get_entries().size(), 2);
}

/**
 * @brief Tests get_entry returns a default LogEntry for out-of-range indices.
 */
TEST_F(LogModelTest, GetEntryOutOfRangeReturnsDefault)
{
    EXPECT_TRUE(m_model.get_entry(0).get_level().isEmpty());
    m_model.add_entry(
        LogEntry(QDateTime::currentDateTime(), "INFO", "A", LogFileInfo("dummy.log", "App")));
    EXPECT_TRUE(m_model.get_entry(-1).get_level().isEmpty());
    EXPECT_TRUE(m_model.get_entry(100).get_level().isEmpty());
}

/**
 * @brief Tests data() returns correct values for DisplayRole.
 */
TEST_F(LogModelTest, DataReturnsCorrectValuesForDisplayRole)
{
    QDateTime now = QDateTime::currentDateTime();
    LogEntry entry(now, "INFO", "TestMsg", LogFileInfo("dummy.log", "AppX"));
    m_model.add_entry(entry);

    QModelIndex idx0 = m_model.index(0, LogModel::Timestamp);
    QModelIndex idx1 = m_model.index(0, LogModel::Level);
    QModelIndex idx2 = m_model.index(0, LogModel::Message);
    QModelIndex idx3 = m_model.index(0, LogModel::AppName);

    EXPECT_EQ(m_model.data(idx0, Qt::DisplayRole).toDateTime(), now);
    EXPECT_EQ(m_model.data(idx1, Qt::DisplayRole).toString(), "INFO");
    EXPECT_EQ(m_model.data(idx2, Qt::DisplayRole).toString(), "TestMsg");
    EXPECT_EQ(m_model.data(idx3, Qt::DisplayRole).toString(), "AppX");
}

/**
 * @brief Tests data() returns correct values for custom roles.
 */
TEST_F(LogModelTest, DataReturnsCorrectValuesForCustomRoles)
{
    QDateTime now = QDateTime::currentDateTime();
    LogEntry entry(now, "DEBUG", "Msg", LogFileInfo("dummy.log", "AppY"));
    m_model.add_entry(entry);

    QModelIndex idx = m_model.index(0, 0);

    EXPECT_EQ(m_model.data(idx, LogModel::TimestampRole).toDateTime(), now);
    EXPECT_EQ(m_model.data(idx, LogModel::LevelRole).toString(), "DEBUG");
    EXPECT_EQ(m_model.data(idx, LogModel::MessageRole).toString(), "Msg");
    EXPECT_EQ(m_model.data(idx, LogModel::AppNameRole).toString(), "AppY");
}

/**
 * @brief Tests headerData returns correct column names.
 */
TEST_F(LogModelTest, HeaderDataReturnsCorrectNames)
{
    EXPECT_EQ(m_model.headerData(LogModel::Timestamp, Qt::Horizontal, Qt::DisplayRole).toString(),
              "Timestamp");
    EXPECT_EQ(m_model.headerData(LogModel::Level, Qt::Horizontal, Qt::DisplayRole).toString(),
              "Level");
    EXPECT_EQ(m_model.headerData(LogModel::Message, Qt::Horizontal, Qt::DisplayRole).toString(),
              "Message");
    EXPECT_EQ(m_model.headerData(LogModel::AppName, Qt::Horizontal, Qt::DisplayRole).toString(),
              "App Name");
}

/**
 * @brief Tests that a profile-specific schema limits and orders displayed columns.
 */
TEST_F(LogModelTest, DynamicColumnsDisplayOnlyConfiguredFields)
{
    const QVector<LogFieldDefinition> columns{
        {LogField::Timestamp, "Timestamp", LogFieldValueType::Timestamp, false, false, true, true},
        {LogField::Message, "Message", LogFieldValueType::Text, true, false, true, true}};
    m_model.set_columns(columns);

    const QDateTime timestamp = QDateTime::currentDateTime();
    LogEntry entry(timestamp, "INFO", "Visible message", LogFileInfo("dummy.log", "HiddenApp"));
    entry.set_parse_metadata(
        "record", 1,
        {{LogField::Timestamp, timestamp}, {LogField::Message, QStringLiteral("Visible message")}});
    m_model.add_entry(entry);

    EXPECT_EQ(m_model.columnCount(), 2);
    EXPECT_EQ(m_model.get_column_field_id(0), LogField::Timestamp);
    EXPECT_EQ(m_model.get_column_field_id(1), LogField::Message);
    EXPECT_EQ(m_model.data(m_model.index(0, 1), Qt::DisplayRole).toString(), "Visible message");
    EXPECT_EQ(m_model.find_column(LogField::Level), -1);
}

/**
 * @brief Tests that custom parser fields can be appended and displayed.
 */
TEST_F(LogModelTest, AppendedCustomColumnDisplaysParsedValue)
{
    m_model.set_columns(
        {{LogField::Message, "Message", LogFieldValueType::Text, true, false, true, true}});
    m_model.append_columns(
        {{"category", "Category", LogFieldValueType::Text, false, false, false, true},
         {LogField::Message, "Message", LogFieldValueType::Text, true, false, true, true}});

    LogEntry entry(QDateTime(), QString(), "Message", LogFileInfo("dummy.log", QString()));
    entry.set_parse_metadata("record", 1,
                             {{LogField::Message, QStringLiteral("Message")},
                              {QStringLiteral("category"), QStringLiteral("network")}});
    m_model.add_entry(entry);

    ASSERT_EQ(m_model.columnCount(), 2);
    EXPECT_EQ(m_model.headerData(1, Qt::Horizontal, Qt::DisplayRole).toString(), "Category");
    EXPECT_EQ(m_model.data(m_model.index(0, 1), Qt::DisplayRole).toString(), "network");
    EXPECT_FALSE(m_model.is_column_sortable(1));
}

/**
 * @brief Tests flags returns correct item flags.
 */
TEST_F(LogModelTest, FlagsReturnsSelectableAndEnabled)
{
    m_model.add_entry(
        LogEntry(QDateTime::currentDateTime(), "INFO", "A", LogFileInfo("dummy.log", "App")));
    QModelIndex idx = m_model.index(0, 0);
    EXPECT_TRUE(m_model.flags(idx).testFlag(Qt::ItemIsSelectable));
    EXPECT_TRUE(m_model.flags(idx).testFlag(Qt::ItemIsEnabled));
}

/**
 * @brief Tests roleNames returns correct mapping.
 */
TEST_F(LogModelTest, RoleNamesReturnsCorrectMapping)
{
    QHash<int, QByteArray> roles = m_model.roleNames();
    EXPECT_EQ(roles.value(LogModel::TimestampRole), "timestamp");
    EXPECT_EQ(roles.value(LogModel::LevelRole), "level");
    EXPECT_EQ(roles.value(LogModel::MessageRole), "message");
    EXPECT_EQ(roles.value(LogModel::AppNameRole), "app_name");
}
