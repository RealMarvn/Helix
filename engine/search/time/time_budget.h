//
// Created by Marvin Becker on 18.12.25.
//

#pragma once

#include <cstdint>

/**
 * @brief Runtime time control for a single search.
 *
 * TimeControl stores all time-related limits for one search invocation.
 * It is initialized once before the search starts and treated as
 * read-only during the search.
 *
 * The structure supports a two-phase termination strategy:
 * - a soft limit, checked only at safe boundaries (e.g. between
 *   iterative deepening iterations), and
 * - a hard limit, enforced immediately inside the search and
 *   quiescence search.
 */
struct TimeControl
{
    /** @brief Soft time limit in milliseconds.
     *
     * The soft limit is used to terminate the search gracefully,
     * typically after completing the current iteration of
     * iterative deepening.
     */
    int soft_ms = 0;

    /** @brief Hard time limit in milliseconds.
     *
     * The hard limit represents an absolute deadline and is
     * enforced immediately inside the search to guarantee
     * timely termination.
     */
    int hard_ms = 0;

    /** @brief Total time budget allocated for the current move.
     *
     * This value represents the maximum amount of time the engine
     * intends to spend on the move, before soft and hard limits
     * are derived.
     */
    int total_ms = 0;

    /** @brief Search start time in milliseconds (monotonic clock). */
    std::int64_t start_ms = 0;

    /** @brief Absolute soft deadline timestamp in milliseconds. */
    std::int64_t soft_deadline_ms = 0;

    /** @brief Absolute hard deadline timestamp in milliseconds. */
    std::int64_t hard_deadline_ms = 0;

    /**
     * @brief Check whether the soft time limit has been reached.
     *
     * @param NOW_MS Current time in milliseconds obtained from a
     *        monotonic clock.
     * @return True if the soft deadline is active and has been reached.
     */
    [[nodiscard]] bool soft_time_up(const std::int64_t NOW_MS) const
    {
        return soft_deadline_ms > 0 && NOW_MS >= soft_deadline_ms;
    }

    /**
     * @brief Check whether the hard time limit has been reached.
     *
     * @param NOW_MS Current time in milliseconds obtained from a
     *        monotonic clock.
     * @return True if the hard deadline is active and has been reached.
     */
    [[nodiscard]] bool hard_time_up(const std::int64_t NOW_MS) const
    {
        return hard_deadline_ms > 0 && NOW_MS >= hard_deadline_ms;
    }
};
