#include "Qt-LogViewer/Services/LocalFileStatus.h"

#include <QDir>
#include <QFileInfo>

/**
 * @file LocalFileStatus.cpp
 * @brief Implements presentation-independent local file inspection.
 */

/**
 * @brief Inspects a local path without applying application-specific file-format rules.
 * @param file_path Local path to inspect.
 * @return Requested path, normalized path, and current availability.
 */
auto QtCommonLib::inspect_local_file(const QString& file_path) -> LocalFileStatus
{
    LocalFileStatus result;
    result.requested_path = file_path;

    if (!file_path.isEmpty())
    {
        const QFileInfo file_info(file_path);
        const QString canonical_path = file_info.canonicalFilePath();
        result.normalized_path = canonical_path.isEmpty()
                                     ? QDir::cleanPath(file_info.absoluteFilePath())
                                     : canonical_path;

        if (!file_info.exists())
        {
            result.availability = LocalFileAvailability::Missing;
        }
        else if (!file_info.isFile())
        {
            result.availability = LocalFileAvailability::NotAFile;
        }
        else if (!file_info.isReadable())
        {
            result.availability = LocalFileAvailability::NotReadable;
        }
        else
        {
            result.availability = LocalFileAvailability::Available;
        }
    }

    return result;
}
