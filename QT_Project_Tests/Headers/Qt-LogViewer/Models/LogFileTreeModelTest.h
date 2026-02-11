#pragma once

#include <gtest/gtest.h>

#include <QObject>

#include "Qt-LogViewer/Models/LogFileTreeModel.h"

/**
 * @file LogFileTreeModelTest.h
 * @brief Test fixture for LogFileTreeModel.
 */
class LogFileTreeModelTest: public ::testing::Test
{
    protected:
        LogFileTreeModelTest() = default;
        ~LogFileTreeModelTest() override = default;

        void SetUp() override;
        void TearDown() override;

        LogFileTreeModel* m_model = nullptr;
};

/**
 * @brief Helper QObject to capture signals from LogFileTreeModel.
 */
class AllSessionsRemovedSpy final: public QObject
{
        Q_OBJECT
    public:
        explicit AllSessionsRemovedSpy(QObject* parent = nullptr): QObject(parent) {}

        int count = 0;

    public slots:
        void on_all_sessions_removed()
        {
            ++count;
        }
};
