#pragma once

#include <gtest/gtest.h>

#include "Qt-LogViewer/Services/LogParsingProfile.h"
#include "QtCommonLib/TestSupport/TestFileSystem.h"

class SessionController;
class SessionManager;
class SessionRepository;
class TestLogRuntime;

/**
 * @file SessionControllerTest.h
 * @brief Declares integration tests for coordinated typed session restoration.
 */

/**
 * @class SessionControllerTest
 * @brief Exercises session restoration across view import, query state, and profile resolution.
 */
class SessionControllerTest: public ::testing::Test
{
    protected:
        /** @brief Creates the controller graph and isolated session storage used by each test. */
        void SetUp() override;

        /** @brief Destroys controllers and removes isolated session storage. */
        void TearDown() override;

        LogParsingProfile m_default_profile{LogParsingProfile::create_default(
            QStringLiteral("{timestamp} {level} {message} {app_name}"),
            QStringLiteral("Session test default"))};
        TestLogRuntime* m_runtime{nullptr};
        SessionController* m_session_controller{nullptr};
        /** @brief Isolated persistence repository used by session import tests. */
        SessionRepository* m_session_repository{nullptr};
        /** @brief Session manager backed by the isolated test repository. */
        SessionManager* m_session_manager{nullptr};
        /** @brief Unique AppConfig subdirectory removed after each test. */
        QString m_session_repository_subdir;
        /** @brief Isolated filesystem owning source and session files created by each test. */
        QtCommonLib::TestFileSystem m_file_system;
};
