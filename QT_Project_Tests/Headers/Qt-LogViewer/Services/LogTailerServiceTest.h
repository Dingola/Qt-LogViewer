#pragma once

#include <gtest/gtest.h>

#include <QSignalSpy>
#include <QTemporaryDir>

#include "Qt-LogViewer/Services/LogTailerService.h"

/**
 * @file LogTailerServiceTest.h
 * @brief Declares the QFileSystemWatcher-based LogTailerService test fixture.
 */
class LogTailerServiceTest: public ::testing::Test
{
    protected:
        /**
         * @brief Creates a temporary file system location and tailer instance.
         */
        void SetUp() override;

        /**
         * @brief Releases the tailer and temporary resources.
         */
        void TearDown() override;

        /**
         * @brief Processes watcher events until a signal spy receives the requested count.
         * @param spy Signal spy collecting entries_available emissions.
         * @param expected_count Required signal count.
         * @return True when the count was observed before timeout.
         */
        [[nodiscard]] auto wait_for_entries(QSignalSpy& spy, int expected_count) const -> bool;

        /**
         * @brief Appends raw UTF-8 text to the temporary log file.
         * @param text Text to append.
         */
        auto append_text(const QString& text) const -> void;

    protected:
        QTemporaryDir m_temporary_directory;
        QString m_file_path;
        LogTailerService* m_tailer_service{nullptr};
        QUuid m_view_id;
};
