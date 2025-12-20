//
// Created by Marvin Becker on 16.12.25.
//
/**
 * @file heuristics.h
 * @brief Search heuristics for move ordering (TT move, MVV-LVA captures, killer moves, history).
 *
 * This header contains lightweight data structures used by the alpha-beta / negamax search
 * to improve move ordering.
 *
 * Why this matters:
 *  - Better ordering => earlier beta cutoffs => fewer nodes searched.
 *
 * Implemented heuristics:
 *  - TT / PV move bonus: if a transposition table move is available, it is searched first.
 *  - MVV-LVA for captures: captures are ordered by victim value and attacker value.
 *  - Killer moves: per-ply memorized quiet moves that previously caused a beta-cutoff.
 *  - History heuristic: global quiet-move statistics accumulated on beta-cutoffs.
 *
 * Notes:
 *  - These heuristics must never affect legality or evaluation correctness; only the order
 *    in which moves are searched.
 *  - Killer/History are updated ONLY on beta-cutoffs and ONLY for quiet moves.
 *  - Side indexing: this module expects a compact side index {0 = White, 1 = Black} when
 *    accessing HistoryTable. In your engine this matches `board.player` (WHITE=0, BLACK=1).
 */

#pragma once

#include <array>
#include <algorithm>
#include <cstdint>

#include "core/move.h"

namespace search::heuristics {

/// Maximum ply supported by the heuristic tables (must be >= maximum search depth in plies).
inline constexpr int HEUR_MAX_PLY = 128;

/// Heuristic score bonuses used for move ordering (larger = searched earlier).
inline constexpr int HEUR_TT_BONUS      = 1000000;
inline constexpr int HEUR_KILLER1_BONUS = 90000;
inline constexpr int HEUR_KILLER2_BONUS = 80000;

/**
 * @brief Returns true if a move is considered a "quiet" move for ordering heuristics.
 *
 * Quiet moves are moves without a capture and without a promotion.
 * Castling is treated as quiet (common engine convention).
 */
inline bool is_quiet(const Move& m)
{
    // Castling counts as quiet!
    return !((m.captured_piece_.piece_type_ != EMPTY) || (m.move_type_ == EN_PASSANT)) && (m.move_type_ != PROMOTION);
}

/**
 * @brief Stores up to two killer moves per ply.
 *
 * A "killer" is a quiet move that previously caused a beta-cutoff at the same ply.
 * During move ordering, killer moves are searched early to increase the chance
 * of early cutoffs in similar tactical situations.
 *
 * Usage:
 *  - On beta-cutoff: if (is_quiet(move)) killers.add(ply, move);
 *  - During ordering: add a fixed bonus if the move equals killer[ply][0] or [ply][1].
 */
struct KillerTable
{
    // Two killer moves per ply.
    std::array<std::array<Move, 2>, HEUR_MAX_PLY> killers_{};

    /// Resets all stored killer moves.
    void clear();

    /**
     * @brief Records a killer move for a given ply.
     *
     * Call this ONLY on a beta-cutoff and ONLY for quiet moves.
     * The newest killer becomes killers[ply][0] and the previous one shifts to [1].
     */
    void add(int ply, const Move& move);

    /// Returns true if @p m equals the primary killer move for @p PLY.
    [[nodiscard]] bool is_killer1(int ply, const Move& move) const;
    /// Returns true if @p m equals the secondary killer move for @p PLY.
    [[nodiscard]] bool is_killer2(int ply, const Move& move) const;
};

/**
 * @brief Global history heuristic table for quiet moves.
 *
 * Stores a score for each (side, from, to) quiet move indicating how often it led
 * to a beta-cutoff. Higher values mean the move is more likely to be tried early.
 *
 * Update rule (typical): history[side][from][to] += depth^2 on beta-cutoff.
 */
struct HistoryTable
{
    // [side][from][to]
    std::array<std::array<std::array<int, 64>, 64>, 2> history_{};

    /// Resets all history values to zero.
    void clear();

    /**
     * @brief Adds a history bonus for a quiet move that caused a beta-cutoff.
     * @param side  Side to move (0 = White, 1 = Black).
     * @param from  Source square index (0..63).
     * @param to    Target square index (0..63).
     * @param depth Remaining search depth at the node where the cutoff occurred.
     */
    void add(int side, int from, int to, int depth);

    /// Returns the stored history value for (SIDE, FROM, TO). Out-of-range returns 0.
    [[nodiscard]] int get(int side, int from, int to) const;
};

/**
 * @brief Sorts a list of pseudo-legal moves for alpha-beta search.
 *
 * Call this right after move generation and before iterating the moves.
 * Uses score_move_heuristic() internally.
 *
 * @param moves    Move container to be sorted in-place.
 * @param tt_move  TT/PV move to prioritize (may be invalid if not found).
 * @param ply      Current ply (0 at root).
 * @param side     Side to move (0 = White, 1 = Black).
 * @param killers  Killer move table.
 * @param history  History heuristic table.
 */
void order_moves(
    PseudoLegalMoves& moves,
    const Move& tt_move,
    int ply,
    int side,
    const KillerTable& killers,
    const HistoryTable& history);

} // namespace search::heuristics
