#pragma once

#include <QTemporaryDir>

class QString;

/**
 * @file TestFileSystem.h
 * @brief Declares an isolated temporary filesystem for tests.
 */

/**
 * @class TestFileSystem
 * @brief Owns a temporary directory and provides safe file helpers below it.
 *
 * Every instance receives an isolated root directory. Relative paths are resolved only below that
 * root, and the complete directory tree is removed when the instance is destroyed.
 */
class TestFileSystem final
{
    public:
        /** @brief Creates a new isolated temporary directory. */
        TestFileSystem();

        /** @brief Removes the owned temporary directory and all of its contents. */
        ~TestFileSystem() = default;

        TestFileSystem(const TestFileSystem&) = delete;
        auto operator=(const TestFileSystem&) -> TestFileSystem& = delete;
        TestFileSystem(TestFileSystem&&) = delete;
        auto operator=(TestFileSystem&&) -> TestFileSystem& = delete;

        /**
         * @brief Reports whether the temporary root was created successfully.
         * @return True when the isolated root directory is available.
         */
        [[nodiscard]] auto is_valid() const -> bool;

        /**
         * @brief Returns the absolute path of the isolated root directory.
         * @return Absolute root path, or an empty string when creation failed.
         */
        [[nodiscard]] auto root_path() const -> QString;

        /**
         * @brief Creates or replaces a UTF-8 text file below the isolated root.
         * @param relative_path Relative file path below the isolated root.
         * @param contents Complete text to write without implicit line terminators.
         * @return Absolute file path on success, or an empty string on failure.
         */
        [[nodiscard]] auto write_text_file(const QString& relative_path,
                                           const QString& contents) const -> QString;

        /**
         * @brief Appends UTF-8 text to a file owned by this filesystem.
         * @param file_path Absolute file path previously obtained from this instance.
         * @param contents Text to append without implicit line terminators.
         * @return True when all encoded bytes were appended successfully.
         */
        auto append_text(const QString& file_path, const QString& contents) const -> bool;

        /**
         * @brief Creates a unique absolute path that does not exist.
         * @param relative_path Relative path providing the directory, base name, and suffix.
         * @return Unique non-existent path below the isolated root, or an empty string on failure.
         */
        [[nodiscard]] auto nonexistent_path(const QString& relative_path) const -> QString;

    private:
        /**
         * @brief Resolves a relative path and rejects paths escaping the isolated root.
         * @param relative_path Relative path supplied by a test.
         * @return Absolute path below the root, or an empty string when invalid.
         */
        [[nodiscard]] auto resolve_path(const QString& relative_path) const -> QString;

        /**
         * @brief Checks whether an absolute path belongs to the isolated root.
         * @param absolute_path Absolute path to validate.
         * @return True when the path is located below the isolated root.
         */
        [[nodiscard]] auto owns_path(const QString& absolute_path) const -> bool;

        /** @brief Temporary directory removed automatically with this object. */
        QTemporaryDir m_root;
};
