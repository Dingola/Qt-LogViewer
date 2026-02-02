#pragma once

#include <gtest/gtest.h>

#include <QDateTime>
#include <QHash>
#include <QString>
#include <QStringList>
#include <QVariant>

#include "Qt-LogViewer/Models/RecentListSchema.h"
#include "Qt-LogViewer/Models/RecentRoles.h"
#include "Qt-LogViewer/Models/SessionTypes.h"

/**
 * @file RecentListSchemaTest.h
 * @brief Test fixture for `RecentListSchema` and `RecentListSchemas` helpers.
 */
class RecentListSchemaTest: public ::testing::Test
{
    protected:
        RecentListSchemaTest() = default;
        ~RecentListSchemaTest() override = default;

        void SetUp() override {}
        void TearDown() override {}
};
