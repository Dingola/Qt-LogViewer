#pragma once

#include <gtest/gtest.h>

#include "QtCommonLib/TestSupport/TestFileSystem.h"

class RecentFilesService;
class SessionRepository;

/**
 * @file RecentFilesServiceTest.h
 * @brief Declares focused tests for the recent-file domain service.
 */

/**
 * @class RecentFilesServiceTest
 * @brief Provides isolated storage for recent-file MRU and persistence tests.
 */
class RecentFilesServiceTest: public ::testing::Test
{
    protected:
        /**
         * @brief Creates an isolated repository and initialized recent-file service.
         */
        void SetUp() override;

        /**
         * @brief Destroys the service and repository owned by the fixture.
         */
        void TearDown() override;

        QtCommonLib::TestFileSystem m_file_system;
        SessionRepository* m_repository{nullptr};
        RecentFilesService* m_service{nullptr};
};
