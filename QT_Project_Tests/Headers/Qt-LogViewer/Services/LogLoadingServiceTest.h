#pragma once

#include <gtest/gtest.h>

#include <QEventLoop>
#include <QObject>
#include <QSignalSpy>
#include <QString>
#include <QTimer>
#include <QVector>

#include "Qt-LogViewer/Services/LogLoadingService.h"

/**
 * @file LogLoadingServiceTest.h
 * @brief Test fixture for LogLoadingService.
 *
 * Covers validation failures, signal emission on async start errors, and retry/backoff
 * configuration.
 */
class LogLoadingServiceTest: public ::testing::Test
{
    protected:
        LogLoadingServiceTest();
        ~LogLoadingServiceTest() override;

        void SetUp() override;
        void TearDown() override;

        /**
         * @brief Waits until a given signal is emitted on an object or timeout.
         * @param sender QObject emitting the signal.
         * @param signal The signal to wait on (pointer-to-member signal).
         * @param timeout_ms Maximum milliseconds to wait.
         * @return True if the signal was emitted before timeout; otherwise false.
         */
        template<typename Sender, typename Signal>
        auto wait_for_signal(Sender* sender, Signal signal, int timeout_ms = 500) -> bool
        {
            bool emitted = false;
            QEventLoop loop;
            QTimer timer;
            timer.setSingleShot(true);

            auto c1 = QObject::connect(sender, signal, &loop, [&]() {
                emitted = true;
                loop.quit();
            });
            auto c2 = QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

            timer.start(timeout_ms);
            loop.exec();

            QObject::disconnect(c1);
            QObject::disconnect(c2);

            return emitted;
        }

        /**
         * @brief Helper to subscribe to `error` and capture last emitted tuple.
         * @param service Service instance.
         * @param out_file_path Ref to store file path from the signal.
         * @param out_message Ref to store error message from the signal.
         * @return Connection handle.
         */
        auto connect_error(LogLoadingService* service, QString& out_file_path,
                           QString& out_message) -> QMetaObject::Connection;

        /**
         * @brief Builds a platform-agnostic path that is guaranteed to not exist.
         * @return Absolute file path that does not exist at the time of call.
         */
        [[nodiscard]] auto make_nonexistent_path() const -> QString;

        LogLoadingService* m_service = nullptr;
};
