/**
 * @file TestFileSystem.cpp
 * @brief Implements the isolated temporary filesystem used by tests.
 */

#include "Qt-LogViewer/TestSupport/TestFileSystem.h"

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QUuid>

/** @brief Creates a new isolated temporary directory. */
TestFileSystem::TestFileSystem()
    : m_root(QDir::temp().filePath(QStringLiteral("qt-logviewer-tests-XXXXXX")))
{}

/**
 * @brief Reports whether the temporary root was created successfully.
 * @return True when the isolated root directory is available.
 */
auto TestFileSystem::is_valid() const -> bool
{
    const bool valid = m_root.isValid();
    return valid;
}

/**
 * @brief Returns the absolute path of the isolated root directory.
 * @return Absolute root path, or an empty string when creation failed.
 */
auto TestFileSystem::root_path() const -> QString
{
    QString path;
    if (m_root.isValid())
    {
        path = QFileInfo(m_root.path()).absoluteFilePath();
    }
    return path;
}

/**
 * @brief Creates or replaces a UTF-8 text file below the isolated root.
 * @param relative_path Relative file path below the isolated root.
 * @param contents Complete text to write without implicit line terminators.
 * @return Absolute file path on success, or an empty string on failure.
 */
auto TestFileSystem::write_text_file(const QString& relative_path,
                                     const QString& contents) const -> QString
{
    const QString file_path = resolve_path(relative_path);
    const QByteArray encoded_contents = contents.toUtf8();
    QString written_path;
    bool writable = !file_path.isEmpty();
    if (writable)
    {
        writable = QDir().mkpath(QFileInfo(file_path).absolutePath());
    }
    if (writable)
    {
        QFile file(file_path);
        writable = file.open(QIODevice::WriteOnly | QIODevice::Truncate) &&
                   file.write(encoded_contents) == encoded_contents.size();
        if (writable)
        {
            written_path = file_path;
        }
    }
    return written_path;
}

/**
 * @brief Appends UTF-8 text to a file owned by this filesystem.
 * @param file_path Absolute file path previously obtained from this instance.
 * @param contents Text to append without implicit line terminators.
 * @return True when all encoded bytes were appended successfully.
 */
auto TestFileSystem::append_text(const QString& file_path, const QString& contents) const -> bool
{
    const QString absolute_path = QFileInfo(file_path).absoluteFilePath();
    const QByteArray encoded_contents = contents.toUtf8();
    bool appended = owns_path(absolute_path) && QFileInfo::exists(absolute_path);
    if (appended)
    {
        QFile file(absolute_path);
        appended = file.open(QIODevice::WriteOnly | QIODevice::Append) &&
                   file.write(encoded_contents) == encoded_contents.size();
    }
    return appended;
}

/**
 * @brief Creates a unique absolute path that does not exist.
 * @param relative_path Relative path providing the directory, base name, and suffix.
 * @return Unique non-existent path below the isolated root, or an empty string on failure.
 */
auto TestFileSystem::nonexistent_path(const QString& relative_path) const -> QString
{
    const QString requested_path = resolve_path(relative_path);
    QString missing_path;
    if (!requested_path.isEmpty())
    {
        const QFileInfo requested_info(requested_path);
        const QString suffix = requested_info.completeSuffix();
        const QString unique_name = QStringLiteral("%1_%2%3").arg(
            requested_info.completeBaseName(), QUuid::createUuid().toString(QUuid::WithoutBraces),
            suffix.isEmpty() ? QString() : QStringLiteral(".%1").arg(suffix));
        missing_path =
            QFileInfo(QDir(requested_info.absolutePath()).filePath(unique_name)).absoluteFilePath();
    }
    return missing_path;
}

/**
 * @brief Resolves a relative path and rejects paths escaping the isolated root.
 * @param relative_path Relative path supplied by a test.
 * @return Absolute path below the root, or an empty string when invalid.
 */
auto TestFileSystem::resolve_path(const QString& relative_path) const -> QString
{
    QString resolved_path;
    if (m_root.isValid() && !relative_path.trimmed().isEmpty() &&
        QDir::isRelativePath(relative_path))
    {
        const QString candidate =
            QFileInfo(QDir(m_root.path()).filePath(relative_path)).absoluteFilePath();
        if (owns_path(candidate))
        {
            resolved_path = candidate;
        }
    }
    return resolved_path;
}

/**
 * @brief Checks whether an absolute path belongs to the isolated root.
 * @param absolute_path Absolute path to validate.
 * @return True when the path is located below the isolated root.
 */
auto TestFileSystem::owns_path(const QString& absolute_path) const -> bool
{
    bool owned = false;
    if (m_root.isValid() && QDir::isAbsolutePath(absolute_path))
    {
        const QString relative_path = QDir(m_root.path()).relativeFilePath(absolute_path);
        owned = !relative_path.isEmpty() && relative_path != QStringLiteral("..") &&
                !relative_path.startsWith(QStringLiteral("../")) &&
                QDir::isRelativePath(relative_path);
    }
    return owned;
}
