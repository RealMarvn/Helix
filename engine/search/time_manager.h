//
// Created by Marvin Becker on 16.12.25.
//
/**
 * @file time_manager.h
 *
 * @brief Time management utilities for chess engine search.
 *
 * This module converts UCI tournament clock information
 * (`wtime`, `btime`, `winc`, `binc`, `movestogo`) into a per-move
 * time budget used by the search.
 *
 * In tournament mode (cutechess / fastchess), engines usually receive:
 *   go wtime <ms> btime <ms> [winc <ms>] [binc <ms>] [movestogo <n>]
 * rather than a fixed `go movetime <ms>` command.
 *
 * The TimeManager computes three values per move:
 *   - total_ms: recommended total budget for the move
 *   - soft_ms:  preferred stop time (after a completed iteration)
 *   - hard_ms:  absolute stop time (must not be exceeded)
 *
 * All time values are expressed in milliseconds.
 */

#pragma once

#include <algorithm>


namespace search::time {

/**
 * @brief Raw UCI time control information.
 *
 * Mirrors the clock-related fields sent by UCI GUIs in tournament mode.
 * All values are expressed in milliseconds.
 */
struct UciTimeControl
{
    int wtime = -1;      ///< Remaining white time (ms)
    int btime = -1;      ///< Remaining black time (ms)
    int winc = 0;        ///< White increment per move (ms)
    int binc = 0;        ///< Black increment per move (ms)
    int movestogo = -1;  ///< Moves until next time control (-1 if unknown)
    int overhead_ms = 20; ///< Safety margin for UI / IO / OS jitter (ms)
    int reserve_ms = -1;  ///< Extra reserve to avoid flagging (ms); if <= 0, a default is used
};

/**
 * @brief Per-move time budget produced by the TimeManager.
 *
 * The search should attempt to stop at a clean boundary around @ref soft_ms
 * (e.g. after a completed iteration), and must stop no later than
 * @ref hard_ms.
 */
struct TimeBudget
{
    int soft_ms = 0;   ///< Soft time limit (ms)
    int hard_ms = 0;   ///< Hard time limit (ms)
    int total_ms = 0;  ///< Total allocated budget for the move (ms)
};

/**
 * @brief Converts UCI tournament clocks into a per-move time budget.
 *
 * Typical usage:
 * @code
 * search::time::UciTimeControl tc;
 * tc.wtime = wtime;
 * tc.btime = btime;
 * tc.winc  = winc;
 * tc.binc  = binc;
 * tc.movestogo = movestogo;
 *
 * auto budget = search::time::TimeManager::compute_budget_ms(side_to_move, tc);
 * @endcode
 */
class TimeManager
{
public:
    /// Compute a per-move time budget from tournament clock information.
    static TimeBudget compute_budget_ms(int side_to_move, const UciTimeControl& tc);

    /// Choose a default reserve (ms) based on remaining time.
    static int default_reserve_ms(int my_time_ms);

    /// Choose a default "moves left" estimate when movestogo is not provided.
    static int default_moves_left(int movestogo);
};

} // namespace search::time
