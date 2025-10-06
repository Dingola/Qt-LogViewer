#pragma once

#include <gtest/gtest.h>

#include <QObject>
#include <QSet>
#include <QString>
#include <QTemporaryFile>
#include <QUuid>
#include <QVector>

#include "Qt-LogViewer/Controllers/LogViewerController.h"

/**
 * @file LogViewerControllerTest.h
 * @brief Test fixture for LogViewerController.
 *
 * This test class covers loading, filtering, view management, and model access for
 * LogViewerController.
 */
class LogViewerControllerTest: public ::testing::Test
{
    protected:
        LogViewerControllerTest();
        ~LogViewerControllerTest() override;

        void SetUp() override;
        void TearDown() override;

        /**
         * @brief Helper to create a QTemporaryFile with given log lines.
         * @param lines The log lines to write.
         * @return Pointer to the created QTemporaryFile (ownership transferred).
         */
        auto create_temp_file(const QVector<QString>& lines) -> QTemporaryFile*;

        LogViewerController* m_controller = nullptr;
        QVector<QTemporaryFile*> m_temp_files;
        QVector<QString> m_temp_file_names;
        QUuid m_view_id;
};
