/**
 * @file LogCacheIdentity.cpp
 * @brief Implements bounded-cost file and parser fingerprinting.
 */

#include "Qt-LogViewer/Services/LogCacheIdentity.h"

#include <QCryptographicHash>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <utility>

#include "Qt-LogViewer/Services/LogParsingProfile.h"

namespace
{
/** Maximum number of bytes sampled at either end of a source file. */
constexpr qint64 k_sample_size = 64 * 1024;

/**
 * Adds a length-delimited field to a digest so adjacent values cannot become
 * ambiguous.
 *
 * @param hash Digest receiving the field.
 * @param value Raw field contents.
 */
auto add_hash_field(QCryptographicHash& hash, const QByteArray& value) -> void
{
    hash.addData(QByteArray::number(value.size()));
    hash.addData(":", 1);
    hash.addData(value);
    hash.addData(";", 1);
}

/**
 * Resolves the stable absolute path used as the source identity.
 *
 * Windows paths are case-folded because the normal filesystem semantics are
 * case-insensitive. An absolute path is retained as a fallback for filesystems
 * that cannot provide a canonical path.
 *
 * @param file_info Existing source-file metadata.
 * @return Canonical or absolute clean path, case-folded on Windows.
 */
[[nodiscard]] auto normalized_identity_path(const QFileInfo& file_info) -> QString
{
    QString path = file_info.canonicalFilePath();
    if (path.isEmpty())
    {
        path = file_info.absoluteFilePath();
    }

    path = QDir::cleanPath(path);
#ifdef Q_OS_WIN
    path = path.toCaseFolded();
#endif
    return path;
}

/**
 * Hashes bounded samples from the beginning and end of a source.
 *
 * Files no larger than one sample are read once. Larger files include both
 * labelled samples, keeping fingerprint cost constant even for multi-gigabyte
 * logs.
 *
 * @param file Open, readable source file positioned at its beginning.
 * @param file_size Source size in bytes.
 * @return SHA-256 digest of the labelled samples, or an empty array if seeking fails.
 */
[[nodiscard]] auto create_sample_hash(QFile& file, qint64 file_size) -> QByteArray
{
    QCryptographicHash hash(QCryptographicHash::Sha256);
    const QByteArray first_sample = file.read(k_sample_size);
    add_hash_field(hash, QByteArrayLiteral("first"));
    add_hash_field(hash, first_sample);

    if (file_size > k_sample_size)
    {
        const qint64 last_offset = qMax<qint64>(0, file_size - k_sample_size);
        if (!file.seek(last_offset))
        {
            return {};
        }

        add_hash_field(hash, QByteArrayLiteral("last"));
        add_hash_field(hash, file.read(k_sample_size));
    }

    return hash.result();
}
}  // namespace

/**
 * @brief Checks whether every identity component has a structurally valid value.
 * @return True when the path, sizes, timestamps, digests and cache key are populated.
 */
auto LogCacheIdentity::is_valid() const -> bool
{
    return !canonical_file_path.isEmpty() && file_size >= 0 && modified_utc_ms >= 0 &&
           sample_sha256.size() == QCryptographicHash::hashLength(QCryptographicHash::Sha256) &&
           parser_sha256.size() == QCryptographicHash::hashLength(QCryptographicHash::Sha256) &&
           cache_key.size() == QCryptographicHash::hashLength(QCryptographicHash::Sha256) * 2;
}

/**
 * @brief Creates a bounded-cost identity for a readable source and parser configuration.
 * @param file_path Source file to identify.
 * @param profile Effective parsing profile whose configuration affects cached results.
 * @return Stable cache identity, or std::nullopt when the source cannot be sampled.
 */
auto LogCacheIdentity::create(const QString& file_path,
                              const LogParsingProfile& profile) -> std::optional<LogCacheIdentity>
{
    const QFileInfo file_info(file_path);
    QFile file(file_info.absoluteFilePath());
    std::optional<LogCacheIdentity> identity;

    if (file_info.exists() && file_info.isFile() && file.open(QIODevice::ReadOnly))
    {
        LogCacheIdentity candidate;
        candidate.canonical_file_path = normalized_identity_path(file_info);
        candidate.file_size = file_info.size();
        candidate.modified_utc_ms = file_info.lastModified().toUTC().toMSecsSinceEpoch();
        candidate.sample_sha256 = create_sample_hash(file, candidate.file_size);

        // Only effective parsing behavior is hashed. Profile names and IDs are
        // intentionally excluded so equivalent profiles can reuse the same
        // generated cache.
        const QByteArray parser_json =
            QJsonDocument(profile.get_configuration().to_json()).toJson(QJsonDocument::Compact);
        QCryptographicHash parser_hash(QCryptographicHash::Sha256);
        add_hash_field(parser_hash, QByteArrayLiteral("parser-schema"));
        add_hash_field(parser_hash, QByteArray::number(LogParsingProfile::SchemaVersion));
        add_hash_field(parser_hash, parser_json);
        candidate.parser_sha256 = parser_hash.result();

        // The namespace marker prevents an identity created by another cache format
        // from being mistaken for a v2 generation even if all remaining components
        // happen to match.
        QCryptographicHash cache_hash(QCryptographicHash::Sha256);
        add_hash_field(cache_hash, QByteArrayLiteral("qt-log-viewer-cache-v2"));
        add_hash_field(cache_hash, candidate.canonical_file_path.toUtf8());
        add_hash_field(cache_hash, QByteArray::number(candidate.file_size));
        add_hash_field(cache_hash, QByteArray::number(candidate.modified_utc_ms));
        add_hash_field(cache_hash, candidate.sample_sha256);
        add_hash_field(cache_hash, candidate.parser_sha256);
        candidate.cache_key = QString::fromLatin1(cache_hash.result().toHex());

        if (candidate.is_valid())
        {
            identity = std::move(candidate);
        }
    }

    return identity;
}
