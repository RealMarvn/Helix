//
// Created by Marvin Becker on 10.02.24.
//
/**
 * @file chess_bot.h
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
#include "./search/heuristics.h"

/**
 * @class ChessBot
 * @brief Core search engine responsible for selecting the best move.
 *
 * The class handles iterative deepening time management, transposition table
 * handling, negamax search with alpha-beta pruning, quiescence search, and
 * move evaluation. It exposes two main search entry points:
 * - generate_best_next_move() for time‑limited search
 * - generate_best_next_move_fixed_depth() for depth‑limited search
 */
class ChessBot {
public:

    /**
     * @brief Performs an iterative deepening search to find the best move for the given board.
     *
     * @param board The chess board to search for the best move.
     * @param time_constraint The time in ms the bot has to search.
     * @return The best move found.
     */
    Move generate_best_next_move(Board& board, int time_constraint);

    /**
     * @brief Performs a fixed depth search to find the best move for the given board.
    *
    * @param board The chess board to search for the best move.
     * @param DEPTH The depth the bot has to reach.
    * @return The best move found.
    */
    Move generate_best_next_move_fixed_depth(Board& board, int DEPTH);

    /**
     * @brief Resets the transposition table to an empty state.
     *
     * Clears all stored entries so the next search starts without reused
     * move suggestions from previous positions.
     */
    void reset_tt();

private:

    /**
     * @brief Transposition table (hash table) for searched positions.
     *
     * Indexed by Zobrist hash modulo tt_size. Stores moves to improve move
     * ordering and prune repeated subtrees during search.
     */
    TranspositionTable tt{1u << 20}; // ~1M entries (power of two)

    /**
     * @brief Killer move heuristic table.
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
     * @brief Timestamp marking the beginning of an iterative deepening search.
     */
    std::chrono::high_resolution_clock::time_point iterative_time_point;

    /**
     * @brief Maximum allowed time for iterative deepening in milliseconds.
     */
    int iterative_time_constraint = 2000;

    /**
     * @brief Checks if the time has elapsed.
     *
     * This function calculates the elapsed time between the iterativeTimePoint time and the current time,
     * and checks if the elapsed time is greater than or equal to the configured time constraint.
     *
     * @return true if the time has elapsed, false otherwise.
     */
    [[nodiscard]] bool is_time_up() const {
        const std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
        const long long elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - iterative_time_point)
                                           .count();
        return elapsed_time >= iterative_time_constraint;
    }

    /**
     * @brief Search for the best next move in a chess game.
     *
     * This function performs a search for the best next move in a given chess board configuration.
     * The search is based on a specified depth, which determines the level of exploration of possible game positions.
     * The function implements the negamax algorithm with alpha-beta pruning to make the search more efficient.
     *
     * @param board The current board configuration.
     * @param DEPTH The depth of the search. Determines the level of exploration of possible game positions.
     *
     * @return The best move found by the search algorithm.
     */
    Move search_next_move(Board& board, int DEPTH);

    /**
     * @brief Performs a search for the best move using the negamax algorithm with alpha-beta pruning.
     *
     * @param board The board object representing the current game state.
     * @param DEPTH The maximum depth for the search.
     * @param alpha The alpha value representing the best lower bound found so far.
     * @param BETA The beta value representing the best upper bound found so far.
     * @param PLY The current ply (half-move) count.
     * @param best_move The reference to the best move found so far.
     * @return The score of the best move found.
     */
    int search(Board& board, int DEPTH, int alpha, int BETA, int PLY, Move& best_move);

    /**
     * Perform a quiescence search on the chess board to evaluate the best move.
     *
     * It only searches the captures to ensure that a piece is not sacrificed by the negamax's reached depth.
     *
     * @param board The current chess board.
     * @param alpha The alpha value representing the lower bound of the search window.
     * @param BETA The beta value representing the upper bound of the search window.
     * @return The best score found by the search.
     */
    int quiescence_search(Board& board, int alpha, int BETA);


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
};
