#include "Qt-LogViewer/Services/LogViewerDropValidatorTest.h"

#include <QFileInfo>
#include <QMimeData>
#include <QSet>
#include <QUrl>

#include "Qt-LogViewer/Services/LogViewerDropValidator.h"

/**
 * @file LogViewerDropValidatorTest.cpp
 * @brief Implements focused tests for drag-and-drop file validation.
 */

/** @test Accepts readable local logs while rejecting invalid, unsupported, and non-file entries. */
TEST_F(LogViewerDropValidatorTest, AcceptsOnlySupportedReadableLocalLogFiles)
{
    const QString log_path = m_file_system.write_text_file(
        QStringLiteral("accepted.log"), QStringLiteral("2026-10-15 INFO Accepted"));
    const QString unsupported_path =
        m_file_system.write_text_file(QStringLiteral("unsupported.csv"), QStringLiteral("value"));
    const QString invalid_session_path = m_file_system.write_text_file(
        QStringLiteral("invalid-session.json"), QStringLiteral(R"({"unrelated":true})"));
    const QString directory_path = QFileInfo(log_path).absolutePath();
    QMimeData mime_data;
    mime_data.setUrls({QUrl::fromLocalFile(log_path), QUrl::fromLocalFile(unsupported_path),
                       QUrl::fromLocalFile(log_path + QStringLiteral(".missing")),
                       QUrl(QStringLiteral("https://example.com/remote.log")),
                       QUrl::fromLocalFile(directory_path),
                       QUrl::fromLocalFile(invalid_session_path)});

    const LogViewerDropValidationResult result = LogViewerDropValidator::validate(mime_data);

    ASSERT_TRUE(result.is_accepted());
    EXPECT_EQ(result.content_type, LogViewerDropContentType::LogFiles);
    ASSERT_EQ(result.accepted_paths.size(), 1);
    EXPECT_EQ(result.accepted_paths.constFirst(), QFileInfo(log_path).canonicalFilePath());
    EXPECT_EQ(result.rejected_entries.size(), 5);
}

/** @test Filters repeated paths and files already present in the active session. */
TEST_F(LogViewerDropValidatorTest, FiltersDuplicateAndAlreadyLoadedLogFiles)
{
    const QString new_path =
        m_file_system.write_text_file(QStringLiteral("new.txt"), QStringLiteral("new"));
    const QString loaded_path =
        m_file_system.write_text_file(QStringLiteral("loaded.log"), QStringLiteral("loaded"));
    QMimeData mime_data;
    mime_data.setUrls({QUrl::fromLocalFile(new_path), QUrl::fromLocalFile(new_path),
                       QUrl::fromLocalFile(loaded_path)});
    const QSet<QString> loaded_files{loaded_path};

    const LogViewerDropValidationResult result =
        LogViewerDropValidator::validate(mime_data, loaded_files);

    ASSERT_TRUE(result.is_accepted());
    ASSERT_EQ(result.accepted_paths.size(), 1);
    EXPECT_EQ(result.accepted_paths.constFirst(), QFileInfo(new_path).canonicalFilePath());
    EXPECT_EQ(result.rejected_entries.size(), 2);
}

/** @test Recognizes a valid session document and rejects ambiguous mixed-content drops. */
TEST_F(LogViewerDropValidatorTest, DistinguishesSessionDocumentsFromLogFiles)
{
    const QString session_path = m_file_system.write_text_file(
        QStringLiteral("session.json"),
        QStringLiteral(R"({"schema_version":1,"name":"Dropped","views":[]})"));
    const QString log_path =
        m_file_system.write_text_file(QStringLiteral("application.log"), QStringLiteral("entry"));
    QMimeData session_mime_data;
    session_mime_data.setUrls({QUrl::fromLocalFile(session_path)});
    QMimeData mixed_mime_data;
    mixed_mime_data.setUrls({QUrl::fromLocalFile(session_path), QUrl::fromLocalFile(log_path)});

    const LogViewerDropValidationResult session_result =
        LogViewerDropValidator::validate(session_mime_data);
    const LogViewerDropValidationResult mixed_result =
        LogViewerDropValidator::validate(mixed_mime_data);

    ASSERT_TRUE(session_result.is_accepted());
    EXPECT_EQ(session_result.content_type, LogViewerDropContentType::SessionFile);
    EXPECT_EQ(session_result.accepted_paths.size(), 1);
    EXPECT_FALSE(mixed_result.is_accepted());
    EXPECT_EQ(mixed_result.content_type, LogViewerDropContentType::None);
    EXPECT_EQ(mixed_result.rejected_entries.size(), 2);
}
