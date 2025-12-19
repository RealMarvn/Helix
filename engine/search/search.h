//
// Created by Marvin Becker on 10.02.24.
//
/**
 * @file search.h
 * @brief Declares the ChessBot class implementing search logic for the engine.
 *
 * The ChessBot class provides iterative deepening, fixed-depth searches,
 * negamax with alpha-beta pruning, quiescence search, evaluation, and a
 * transposition-table interface. It forms the core decision-making component
 * of the engine.
 */

#pragma once
#include <chrono>
#include <memory>
#include <climits>
#include <array>

#include "../movement/move_gen.h"
#include "./tt.h"
#include "time/time_manager.h"
#include "./search/heuristics.h"
#include "time/search_constraints.h"

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

public:

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

private:

    SearchConstraints constraint;

    long long nodes_searched = 0;

    /**
     * @brief Transposition table (hash table) for searched positions.
     *
     * Indexed by Zobrist hash modulo tt_size. Stores moves to improve move
     * ordering and prune repeated subtrees during search.
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
     * search was aborted due to a hard termination condition.
     *
     * @param board Current board position.
     * @param DEPTH Fixed depth for this root search.
     * @param move Output parameter receiving the best move found
     *             if the search completes successfully.
     * @return True if the search was aborted, false otherwise.
     */
    bool root_search(Board& board, int DEPTH, Move& move);

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
     * @param DEPTH Remaining search depth.
     * @param alpha Alpha bound of the search window.
     * @param BETA Beta bound of the search window.
     * @param PLY Current ply (half-move) depth from the root.
     * @param best_move Output parameter for the best move at root ply.
     * @return SearchResult containing the score and abort status.
     */
    SearchResult negamax(Board& board, int DEPTH, int alpha, int BETA, int PLY, Move& best_move);

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
     * @param BETA Beta bound of the quiescence window.
     * @return SearchResult containing the score and abort status.
     */
    SearchResult quiescence(Board& board, int alpha, int BETA);


    /**
     * @brief Resets all search-related state before starting a new root search.
     *
     * This function prepares the engine for a fresh search by resetting all
     * data structures that are local to a single root search:
     *  - Clears the transposition table search state (via tt.new_search()).
     *  - Clears the killer move table.
     *  - Clears the history heuristic table.
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
    [[nodiscard]] bool hard_stop()const;

    static Move pick_fallback_root_move(Board& board);
};
