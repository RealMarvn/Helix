//
// Created by Marvin Becker on 10.02.24.
//

/**
 * @file search.h
 * @brief Search interface for the chess engine.
 *
 * Declares the ChessBot class which implements the engine's game-tree search:
 * - iterative deepening and fixed-depth search,
 * - negamax with alpha-beta pruning,
 * - quiescence search,
 * - move ordering (TT move, killers, history),
 * - integration with the Transposition Table.
 */

#pragma once
#include <chrono>
#include <memory>
#include <climits>
#include <cstdint>
#include <atomic>

#include "../movement/move_gen.h"
#include "./tt.h"
#include "./search/heuristics.h"
#include "time/search_constraints.h"
#include "search/debug_print.h"

/**
 * @class ChessBot
 * @brief Core search engine responsible for selecting the best move.
 *
 * ChessBot implements the complete game-tree search logic of the engine.
 * It supports:
 *  - iterative deepening with time management,
 *  - fixed-depth searches,
 *  - negamax search with alpha-beta pruning,
 *  - quiescence search,
 *  - transposition tables,
 *  - killer and history heuristics.
 *
 * Search termination is controlled via SearchConstraints, which may
 * include time limits, node limits, or fixed depth. Hard termination
 * conditions are propagated explicitly through the search to avoid
 * using incomplete results.
 */
class ChessBot {

public:

    /** @brief Debug verbosity level used by the search debug printer. */
    enum class DebugLevel : uint8_t
    {
        BASIC,  // Stop reasons.
        MEDIUM, // TT stats, search health.
        VERBOSE // move ordering, extras.
    };

    /** @brief Reason why a search terminated (used for reporting/debugging). */
    enum StopReason : uint8_t
    {
        NONE,
        STOP_FLAG,  // Stop flag canceled the search.
        HARD_TIME,  // Hard time-limit was reached.
        NODE_LIMIT, // Node-limit was reached.
        SOFT_TIME   // Soft time-limit was reached.
    };

    /**
     * @brief Entry point for starting a new search.
     *
     * Executes a search according to the provided SearchConstraints.
     * Depending on the selected search mode, this function performs
     * either a fixed-depth search or an iterative deepening search
     * with time and/or node limits.
     *
     * @param board Current board position to search from.
     * @param config Immutable search constraints controlling termination.
     * @return Best move found by the completed search.
     */
    Move think(Board board, SearchConstraints config);

    /**
     * @brief Resets the transposition table to an empty state.
     *
     * Clears all stored entries so the next search starts without reused
     * move suggestions from previous positions.
     */
    void reset_tt();

    /** @brief Request termination of the currently running search (thread-safe). */
    void request_stop() { stop_requested.store(true, std::memory_order_relaxed); }

    /** @brief Turn debug logs on and off. */
    void set_debug_enabled(const bool ON) { debug.enabled = ON; }

    /** @brief Set the debug level. */
    void set_debug_level(const DebugLevel LVL) { debug.level = LVL; }

private:

    /**
     * @brief Config object used to hold the debug options.
     *
     * Bundles the level and the flag for debugging.
     */
    struct DebugConfig
    {
        bool enabled = false;
        DebugLevel level = DebugLevel::BASIC;
    };

    /**
     * @brief Result object returned by recursive search functions.
     *
     * SearchResult bundles the evaluated score together with an
     * explicit abort flag. The abort flag is used to signal hard
     * termination conditions (e.g. time or node limits) and is
     * propagated through the negamax recursion to prevent using
     * incomplete evaluations.
     */
    struct SearchResult
    {
        int score = 0;
        bool aborted = false;
    };

    /** @brief Active constraints for the current search (time/nodes/depth). */
    SearchConstraints constraint;

    /** @brief Number of nodes explored in the last (main) search. */
    long long nodes = 0;

    /** @brief Number of nodes explored in quiescence search during the last search. */
    long long qnodes = 0;

    /** @brief Maximum selective depth (seldepth) reached in the last search. */
    int seldepth = 0;

    /** @brief Number of times a TT probe returned a usable score/bound. */
    int tt_returns = 0;

    /** @brief The current debug config. */
    DebugConfig debug;

    /** @brief Stop reason of the last completed/aborted search. */
    StopReason stop_reason = StopReason::NONE;

    /** @brief An atomic flag to stop. */
    std::atomic<bool> stop_requested{false};

    /**
     * @brief Transposition Table (hash table) for previously searched positions.
     *
     * Probed during search to reuse previously computed scores (exact values or
     * bounds) and to get a good move for move ordering.
     */
    TranspositionTable tt{1u << 20}; // ~1M entries (power of two)

    /**
     * @brief Killer moves heuristic table.
     *
     * Stores up to two quiet moves per ply that previously caused a beta-cutoff.
     * Used only for move ordering (does not affect correctness).
     */
    search::heuristics::KillerTable killers;

    /**
     * @brief History heuristic table.
     *
     * Accumulates quiet-move statistics (side, from, to) on beta-cutoffs.
     * Higher values indicate moves that tend to cause cutoffs and should be
     * searched earlier.
     */
    search::heuristics::HistoryTable history;

    /**
     * @brief Executes a single root-level search at a fixed depth.
     *
     * Performs a complete negamax search from the root position
     * to the specified depth. The function returns whether the
     * search was aborted due to a hard termination condition and
     * the corresponding search score.
     *
     * @param board Current board position.
     * @param depth Fixed depth for this root search.
     * @param move Output parameter receiving the best move found
     *             if the search completes successfully.
     * @return The Search-Result with the score and if it got aborted.
     */
    SearchResult root_search(Board& board, int depth, Move& move);

    /**
     * @brief Performs an iterative deepening search.
     *
     * Repeatedly executes root searches with increasing depth until
     * a soft or hard termination condition is reached. Only results
     * from fully completed iterations are considered valid and may
     * update the final best move.
     *
     * @param board Current board position.
     * @return Best move from the deepest fully completed iteration.
     */
    Move iterative_deepening(Board& board);

    /**
     * @brief Negamax search with alpha-beta pruning.
     *
     * Recursively explores the game tree using the negamax formulation
     * with alpha-beta pruning. Hard termination conditions are checked
     * inside the search and propagated explicitly using SearchResult
     * to ensure that incomplete results are never used.
     *
     * @param board Current board position.
     * @param depth Remaining search depth.
     * @param alpha Alpha bound of the search window.
     * @param beta Beta bound of the search window.
     * @param ply Current ply (half-move) depth from the root.
     * @param best_move Output parameter for the best move at root ply.
     * @return SearchResult containing the score and abort status.
     */
    SearchResult negamax(Board& board, int depth, int alpha, int beta, int ply, Move& best_move);

    /**
     * @brief Quiescence search to resolve tactical instability.
     *
     * Extends the leaf nodes of the main search by exploring only
     * forcing moves (captures and checks) to avoid the horizon effect.
     * Hard termination conditions are checked and propagated in the
     * same way as in the main negamax search.
     *
     * @param board Current board position.
     * @param alpha Alpha bound of the quiescence window.
     * @param beta Beta bound of the quiescence window.
     * @param ply Current ply (half-move) depth from root.
     * @return SearchResult containing the score and abort status.
     */
    SearchResult quiescence(Board& board, int alpha, int beta, int ply);


    /**
     * @brief Resets all search-related state before starting a new root search.
     *
     * This function prepares the engine for a fresh search by resetting all
     * data structures that are local to a single root search:
     *  - Advances the transposition table generation (via tt.new_search()).
     *  - Clears the killer move table.
     *  - Clears the history heuristic table.
     *  - Clears the current stats of the nodes, qnodes, seldepth, stop-reason,
     *  and the tt returns.
     *
     * It must be called exactly once at the beginning of a new search
     * (e.g. before iterative deepening or fixed-depth search starts),
     * and must NOT be called inside the recursive search itself.
     *
     * Keeping this logic in a dedicated function avoids code duplication
     * and ensures consistent initialization of all search heuristics.
     */
    void reset_search_state();

    /**
     * @brief Check whether a hard termination condition has been reached.
     *
     * Hard termination conditions include exceeding the hard time
     * limit or the configured node limit. This function must be
     * safe to call at any point inside the search.
     *
     * @return True if the search must terminate immediately.
     */
    [[nodiscard]] bool hard_stop();

    /** @brief Clear a previously requested stop before starting a new search. */
    void clear_stop() { stop_requested.store(false, std::memory_order_relaxed); }

    /** @brief Update node counters and selective depth during negamax. */
    void updateStats(const int PLY)
    {
        ++nodes;
        if (PLY > seldepth)
            seldepth = PLY;
    }

    /** @brief Print UCI "info" line (depth/score/nodes/time/PV). */
    void print_info(int depth, int score, const Move& pv_move, long long start_time_ms) const;

    /** @brief Print additional debug information based on the debug config. */
    void print_debug(Board& board, int depth, int score, long long start_time_ms) const;

    /**
     * @brief Debug helper functions requiring read-only access to internal search state.
     *
     * These functions are declared as friends to allow detailed inspection of
     * the search internals (statistics, transposition table state, move ordering)
     * without exposing this data through the public API.
     *
     * They are intended strictly for debugging and diagnostics and do not
     * influence the search logic.
     */
    friend void search::debug::print_health(const ChessBot& bot);
    friend void search::debug::print_tt(const ChessBot& bot);
    friend void search::debug::print_root_ordering(const ChessBot& bot, Board& board);
    friend void search::debug::print_pv(const ChessBot& bot, const Board& board);

    /**
     * @brief Convert a StopReason enum value into a stable C-string.
     *
     * Used for UCI output and debug logging. The returned string is
     * statically allocated and valid for the entire program lifetime.
     *
     * @param r Stop reason enum value.
     * @return Null-terminated string representation.
     */
    static const char* stop_reason_to_cstr(const decltype(ChessBot::stop_reason) r)
    {
        switch (r)
        {
        case ChessBot::STOP_FLAG: return "stop_flag";
        case ChessBot::HARD_TIME: return "hard_time";
        case ChessBot::SOFT_TIME: return "soft_time";
        case ChessBot::NODE_LIMIT: return "node_limit";
        default: return "none";
        }
    }

};
