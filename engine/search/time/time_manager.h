//
// Created by Marvin Becker on 16.12.25.
//
#pragma once

#include <algorithm>
#include <cstdint>
#include "time_budget.h"
#include "search_constraints.h"
#include "uci_time_control.h"


namespace search::time
{

/**
 * @brief Time management utilities for the chess search.
 *
 * TimeManager converts raw UCI time control information (UciTimeControl)
 * into an internal time budget (TimeControl) and provides a monotonic
 * time source for the search.
 *
 * Typical usage:
 *  - The UCI/driver layer parses the UCI "go" command into SearchConstraints
 *    and UciTimeControl.
 *  - compute_budget(...) is used to derive a per-move TimeControl.
 *  - init_search(...) arms the TimeControl with start and deadline timestamps
 *    directly before the search starts.
 */
class TimeManager
{
public:
    /**
     * @brief Compute a per-move time budget from raw UCI time control.
     *
     * This function implements the time management policy for tournament
     * clocks (remaining time, increment, movestogo, overhead, reserve).
     *
     * @param side_to_move Side to move (0 = white, 1 = black).
     * @param tc Raw UCI time control information.
     * @return Computed per-move TimeControl (soft/hard/total limits in ms).
     */
    static TimeControl compute_budget(int side_to_move, const UciTimeControl& tc);

    /**
     * @brief Arm the time budget with start time and absolute deadlines.
     *
     * This function must be called directly before starting the search.
     * For non-time-based searches (e.g. fixed depth), the budget should be
     * left unarmed (all deadlines set to zero).
     *
     * @param search_limits Search constraints containing the time budget.
     */
    static void init_search(SearchConstraints& search_limits);



    /**
     * @brief Monotonic time source for the search.
     *
     * @return Current time in milliseconds from a monotonic clock.
     */
    [[nodiscard]] static std::int64_t now_ms();

private:
    /**
     * @brief Choose a default reserve (ms) based on remaining time.
     *
     * @param my_time_ms Remaining time for the side to move (ms).
     * @return Reserve time in milliseconds.
     */
    static int default_reserve_ms(int my_time_ms);

    /**
     * @brief Choose a default "moves left" estimate.
     *
     * Used when movestogo is not provided by the GUI.
     *
     * @param movestogo Moves until the next time control, or negative if unknown.
     * @return Estimated remaining moves used for time allocation.
     */
    static int default_moves_left(int movestogo);
};

} // namespace search::time
