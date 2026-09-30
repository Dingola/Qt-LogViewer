#pragma once

#include <gtest/gtest.h>

#include <QTemporaryDir>

/**
 * @file LogCacheCatalogTest.h
 * @brief Declares the isolated fixture used by persistent cache schema tests.
 */

/**
 * @class LogCacheCatalogTest
 * @brief Provides a unique temporary filesystem root for each cache test.
 */
class LogCacheCatalogTest: public ::testing::Test
{
    protected:
        /** @brief Temporary directory containing source logs and SQLite databases for one test. */
        QTemporaryDir m_temporary_directory;
};
