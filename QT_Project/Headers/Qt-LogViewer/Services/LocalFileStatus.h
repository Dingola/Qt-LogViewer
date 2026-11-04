#pragma once

#include <QString>

/**
 * @file LocalFileStatus.h
 * @brief Declares a reusable, presentation-independent local file status contract.
 */

/** @brief Owns reusable, application-independent Qt core utilities. */
namespace QtCommonLib
{
/**
 * @enum LocalFileAvailability
 * @brief Describes why a requested local path can or cannot be consumed as a file.
 */
enum class LocalFileAvailability
{
    Available,
    Missing,
    NotReadable,
    NotAFile
};

/**
 * @struct LocalFileStatus
 * @brief Captures the requested path, its stable normalized form, and current availability.
 */
struct LocalFileStatus {
        /** @brief Path supplied by the caller without modification. */
        QString requested_path;
        /** @brief Canonical existing path or cleaned absolute fallback for a missing path. */
        QString normalized_path;
        /** @brief Current filesystem availability of the requested path. */
        LocalFileAvailability availability{LocalFileAvailability::Missing};
};

/**
 * @brief Inspects a local path without applying application-specific file-format rules.
 * @param file_path Local path to inspect.
 * @return Requested path, normalized path, and current availability.
 */
[[nodiscard]] auto inspect_local_file(const QString& file_path) -> LocalFileStatus;
}  // namespace QtCommonLib
