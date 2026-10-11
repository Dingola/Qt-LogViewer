#pragma once

#include <gtest/gtest.h>

#include "Qt-LogViewer/Services/SessionRepository.h"
#include "QtCommonLib/TestSupport/TestFileSystem.h"

/**
 * @file SessionRepositoryTest.h
 * @brief Test fixture for SessionRepository.
 */
class SessionRepositoryTest: public ::testing::Test
{
    protected:
        SessionRepositoryTest() = default;
        ~SessionRepositoryTest() override = default;

        void SetUp() override;
        void TearDown() override;

        SessionRepository* m_repo = nullptr;
        /** @brief Absolute sessions directory owned by the isolated test filesystem. */
        QString m_sessions_directory;
        /** @brief Isolated filesystem owning external session files used by tests. */
        QtCommonLib::TestFileSystem m_file_system;
};
