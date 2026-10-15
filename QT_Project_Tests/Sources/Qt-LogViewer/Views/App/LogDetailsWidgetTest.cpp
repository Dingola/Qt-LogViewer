/**
 * @file LogDetailsWidgetTest.cpp
 * @brief Implements focused tests for the structured log-details inspector.
 */

#include "Qt-LogViewer/Views/App/LogDetailsWidgetTest.h"

#include <QLabel>
#include <QPlainTextEdit>
#include <QTableWidget>
#include <QToolButton>

#include "Qt-LogViewer/Models/LogEntry.h"
#include "Qt-LogViewer/Models/LogFileInfo.h"
#include "Qt-LogViewer/Views/App/LogDetailsWidget.h"

/** @test Verifies structured, raw, source, and custom-field presentation. */
TEST_F(LogDetailsWidgetTest, PresentsCompleteEntryAndClearsSelection)
{
    LogEntry entry(
        QDateTime::fromString(QStringLiteral("2026-10-14T18:42:13.284"), Qt::ISODateWithMs),
        QStringLiteral("ERROR"), QStringLiteral("Database connection timed out"),
        LogFileInfo(QStringLiteral("D:/Logs/application.log"), QStringLiteral("ExampleApp")));
    entry.set_parse_metadata(QStringLiteral("raw database record"), 428,
                             {{QStringLiteral("category"), QStringLiteral("database")},
                              {QStringLiteral("thread"), QStringLiteral("worker-3")}});
    entry.set_source_range(18240, 117);

    LogDetailsWidget widget;
    widget.set_log_entry(entry);

    const auto* message = widget.findChild<QPlainTextEdit*>(QStringLiteral("plainTextEditMessage"));
    const auto* raw = widget.findChild<QPlainTextEdit*>(QStringLiteral("plainTextEditRaw"));
    const auto* fields = widget.findChild<QTableWidget*>(QStringLiteral("tableWidgetFields"));
    const auto* severity = widget.findChild<QLabel*>(QStringLiteral("labelSeverityBadge"));
    const auto* copy_all = widget.findChild<QToolButton*>(QStringLiteral("toolButtonCopyAll"));
    ASSERT_NE(message, nullptr);
    ASSERT_NE(raw, nullptr);
    ASSERT_NE(fields, nullptr);
    ASSERT_NE(severity, nullptr);
    ASSERT_NE(copy_all, nullptr);
    EXPECT_TRUE(widget.has_log_entry());
    EXPECT_EQ(message->toPlainText(), QStringLiteral("Database connection timed out"));
    EXPECT_EQ(raw->toPlainText(), QStringLiteral("raw database record"));
    EXPECT_EQ(severity->text(), QStringLiteral("ERROR"));
    EXPECT_EQ(severity->property("severity").toString(), QStringLiteral("error"));
    EXPECT_EQ(fields->rowCount(), 2);
    EXPECT_TRUE(copy_all->isEnabled());

    widget.clear_log_entry();

    EXPECT_FALSE(widget.has_log_entry());
    EXPECT_TRUE(message->toPlainText().isEmpty());
}
