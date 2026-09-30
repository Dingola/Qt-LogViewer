#pragma once

#include <QByteArray>
#include <QString>
#include <optional>

class LogParsingProfile;

/**
 * @file LogCacheIdentity.h
 * @brief Declares the reusable identity of one parsed source-file revision.
 */

/**
 * @struct LogCacheIdentity
 * @brief Combines a cheap file fingerprint with the effective parser
 * configuration.
 */
struct LogCacheIdentity {
        /** @brief Canonical absolute source path, case-folded on Windows. */
        QString canonical_file_path;

        /** @brief Source size in bytes at the time the identity was created. */
        qint64 file_size{-1};

        /** @brief Source modification time as milliseconds since the UTC epoch. */
        qint64 modified_utc_ms{-1};

        /** @brief SHA-256 digest of bounded samples from the start and end of the
         * source. */
        QByteArray sample_sha256;

        /** @brief SHA-256 digest of the effective parser schema and configuration. */
        QByteArray parser_sha256;

        /** @brief Hex-encoded SHA-256 key derived from all preceding identity
         * components. */
        QString cache_key;

        /**
         * @brief Checks whether every identity component has a structurally valid
         * value.
         * @return True when the path, sizes, timestamps, digests and cache key are
         * populated.
         */
        [[nodiscard]] auto is_valid() const -> bool;

        /**
         * @brief Fingerprints a readable file and parser profile.
         * @param file_path Source file to identify.
         * @param profile Effective parsing profile.
         *
         * At most 64 KiB are read from both the beginning and end of the file. The
         * parser hash uses the effective configuration rather than profile display
         * metadata, allowing equivalent profiles to share a cache generation.
         *
         * @return Stable cache identity, or std::nullopt when the file cannot be
         * sampled.
         */
        [[nodiscard]] static auto create(const QString& file_path, const LogParsingProfile& profile)
            -> std::optional<LogCacheIdentity>;
};
