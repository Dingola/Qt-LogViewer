#pragma once

#include <QElapsedTimer>
#include <QEventLoop>
#include <QTimer>
#include <algorithm>
#include <chrono>
#include <concepts>
#include <functional>
#include <limits>

/**
 * @file QtTestAwait.h
 * @brief Declares event-aware predicate waiting for asynchronous Qt tests.
 */

/**
 * @class QtTestAwait
 * @brief Provides bounded predicate waiting while continuing to process Qt events.
 *
 * The helper is independent of Qt Test and can therefore be shared by test projects that only
 * require Qt Core event processing.
 */
class QtTestAwait final
{
    public:
        QtTestAwait() = delete;

        /**
         * @brief Processes Qt events until a predicate succeeds or the timeout expires.
         * @tparam Predicate Callable whose result is convertible to bool.
         * @param predicate State predicate evaluated immediately and after every polling interval.
         * @param timeout Maximum duration to wait; negative durations behave like zero.
         * @param poll_interval Maximum delay between predicate evaluations; values below one
         * millisecond are clamped to one millisecond.
         * @return True when the predicate succeeds before or at the timeout boundary.
         */
        template<typename Predicate>
            requires std::predicate<Predicate&>
        [[nodiscard]] static auto wait_until(
            Predicate&& predicate, std::chrono::milliseconds timeout = std::chrono::seconds(5),
            std::chrono::milliseconds poll_interval = std::chrono::milliseconds(10)) -> bool
        {
            const qint64 timeout_ms = std::max<qint64>(0, timeout.count());
            const qint64 poll_interval_ms = std::max<qint64>(1, poll_interval.count());
            QElapsedTimer elapsed_timer;
            elapsed_timer.start();
            bool satisfied = static_cast<bool>(std::invoke(predicate));

            while (!satisfied && elapsed_timer.elapsed() < timeout_ms)
            {
                const qint64 remaining_ms = timeout_ms - elapsed_timer.elapsed();
                const qint64 wait_duration_ms = std::min(poll_interval_ms, remaining_ms);
                const int timer_interval_ms = static_cast<int>(
                    std::min<qint64>(wait_duration_ms, std::numeric_limits<int>::max()));
                QEventLoop event_loop;
                QTimer::singleShot(timer_interval_ms, &event_loop, &QEventLoop::quit);
                event_loop.exec(QEventLoop::AllEvents);
                satisfied = static_cast<bool>(std::invoke(predicate));
            }

            return satisfied;
        }
};
