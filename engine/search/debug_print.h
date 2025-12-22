//
// Created by Marvin Becker on 20.12.25.
//

/**
 * @file debug_print.h
 * @brief Debug and diagnostic output helpers for the search module.
 *
 * This header declares debug-printing functions used to inspect the internal
 * state of the search during or after a search run. These functions are
 * intentionally separated from the core search logic and are enabled only
 * when debug output is requested.
 *
 * All functions are read-only with respect to the engine state and must not
 * influence the actual search behavior.
 */

#pragma once

#include <cstdint>

// Forward declarations to avoid heavy includes in this header.
class Board;
class ChessBot;

/**
 * @brief Debug-only helper functions for inspecting search internals.
 *
 * These functions are declared as friends of ChessBot where necessary in order
 * to access internal search state (statistics, transposition table contents,
 * move ordering, principal variation, etc.).
 *
 * They are intended strictly for diagnostics and verbose logging and must not
 * modify engine state or affect search results.
 */
namespace search::debug
{
    // High-level entry points used by ChessBot::print_debug().

    /**
     * @brief Print a high-level health summary of the last search.
     *
     * Includes statistics such as explored nodes, selective depth, stop reason,
     * and transposition table usage.
     */
    void print_health(const ChessBot& bot);

    /**
     * @brief Print diagnostic information about the Transposition Table.
     */
    void print_tt(const ChessBot& bot);

    // VERBOSE helpers (root-only, potentially expensive).

    /**
     * @brief Print the move ordering at the root position.
     *
     * Intended for verbose debugging to understand how heuristics (TT move,
     * killers, history, captures) influence the root move order.
     */
    void print_root_ordering(const ChessBot& bot, Board& board);

    /**
     * @brief Print the principal variation (PV) of the last completed search.
     */
    void print_pv(const ChessBot& bot, const Board& board);
}
