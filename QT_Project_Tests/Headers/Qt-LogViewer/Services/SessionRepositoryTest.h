#pragma once

#include <gtest/gtest.h>

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QString>
#include <QUuid>

#include "Qt-LogViewer/Services/SessionRepository.h"

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

        /**
         * @brief Returns a unique subdirectory under AppConfigLocation for isolation.
         */
        [[nodiscard]] auto get_unique_subdir() const -> QString;

        /**
         * @brief Returns the absolute base directory path used by the repository.
         */
        [[nodiscard]] auto get_base_dir() const -> QString;

        /**
         * @brief Recursively deletes a directory tree. Returns true on success.
         */
        static auto remove_dir_recursive(const QString& dir_path) -> bool;

        SessionRepository* m_repo = nullptr;
        QString m_subdir;
};
