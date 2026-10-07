/**
 * @file QtTestAwaitTest.cpp
 * @brief Implements tests for event-aware predicate waiting.
 */

#include "Qt-LogViewer/TestSupport/QtTestAwaitTest.h"

#include <QElapsedTimer>
#include <QTimer>
#include <chrono>

#include "Qt-LogViewer/TestSupport/QtTestAwait.h"

/** @test Verifies that an already satisfied predicate is evaluated exactly once. */
TEST_F(QtTestAwaitTest, ReturnsImmediatelyForSatisfiedPredicate)
{
    int invocation_count = 0;

    const bool satisfied = QtTestAwait::wait_until([&invocation_count]() {
        invocation_count += 1;
        return true;
    });

    EXPECT_TRUE(satisfied);
    EXPECT_EQ(invocation_count, 1);
}

/** @test Verifies that Qt events are processed while awaiting a predicate. */
TEST_F(QtTestAwaitTest, ProcessesEventsUntilPredicateSucceeds)
{
    bool ready = false;
    QTimer::singleShot(20, [&ready]() { ready = true; });

    const bool satisfied = QtTestAwait::wait_until(
        [&ready]() { return ready; }, std::chrono::milliseconds(500), std::chrono::milliseconds(5));

    EXPECT_TRUE(satisfied);
    EXPECT_TRUE(ready);
}

/** @test Verifies that an unsatisfied predicate returns false after the configured timeout. */
TEST_F(QtTestAwaitTest, ReturnsFalseAfterTimeout)
{
    int invocation_count = 0;
    QElapsedTimer elapsed_timer;
    elapsed_timer.start();

    const bool satisfied = QtTestAwait::wait_until(
        [&invocation_count]() {
            invocation_count += 1;
            return false;
        },
        std::chrono::milliseconds(30), std::chrono::milliseconds(5));

    EXPECT_FALSE(satisfied);
    EXPECT_GT(invocation_count, 1);
    EXPECT_GE(elapsed_timer.elapsed(), 30);
}

/** @test Verifies that a zero timeout still performs the required initial evaluation. */
TEST_F(QtTestAwaitTest, EvaluatesPredicateOnceForZeroTimeout)
{
    int invocation_count = 0;

    const bool satisfied = QtTestAwait::wait_until(
        [&invocation_count]() {
            invocation_count += 1;
            return false;
        },
        std::chrono::milliseconds(0));

    EXPECT_FALSE(satisfied);
    EXPECT_EQ(invocation_count, 1);
}
