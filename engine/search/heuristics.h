//
// Created by Marvin Becker on 16.12.25.
//

/**
 * @file heuristics.h
 * @brief Move-ordering heuristics used by the alpha-beta / negamax search.
 *
 * This header provides lightweight helper tables and routines that influence
 * only the order in which moves are searched (never legality or evaluation).
 *
 * Why this matters:
 *  - Better move ordering increases the likelihood of early beta cutoffs,
 *    which reduces the number of visited nodes for the same search depth.
 *
 * Implemented heuristics (typical engine conventions):
 *  - TT / PV move: if a transposition table move is available, search it first.
 *  - MVV-LVA for captures: order captures by victim value then attacker value.
 *  - Killer moves: per-ply remembered quiet moves that previously caused a cutoff.
 *  - History heuristic: global statistics for quiet moves that caused cutoffs.
 *
 * Notes:
 *  - Killer/History updates are performed ONLY on beta cutoffs and ONLY for quiet moves.
 *  - Side indexing: this module assumes a compact side index {0 = White, 1 = Black}.
 */

#pragma once

#include <array>
#include <algorithm>
#include <cstdint>

#include "core/move.h"

namespace search::heuristics {

/// Maximum ply supported by the heuristic tables (ply = half-move from the root).
/// Must be >= the maximum selective search depth (in plies) used by the engine.
inline constexpr int HEUR_MAX_PLY = 128;

/// Heuristic score bonuses used for move ordering (larger = searched earlier).
/// Relative ordering matters; absolute values are arbitrary as long as tiers don't overlap unintentionally.
inline constexpr int HEUR_TT_BONUS      = 1000000;
inline constexpr int HEUR_KILLER1_BONUS = 90000;
inline constexpr int HEUR_KILLER2_BONUS = 80000;

/**
 * @brief Returns true if a move is considered "quiet" for ordering heuristics.
 *
 * A quiet move is a non-capture and non-promotion move. Many engines also treat
 * castling as quiet, because it is neither a capture nor a promotion and is
 * commonly handled by the quiet-move heuristics (killers/history).
 */
inline bool is_quiet(const Move& m)
{
    // Castling is treated as quiet (engine convention).
    return !((m.captured_piece_.piece_type_ != EMPTY) || (m.move_type_ == EN_PASSANT)) && (m.move_type_ != PROMOTION);
}

/**
 * @brief Stores up to two killer moves per ply.
 *
 * A "killer" is a quiet move that previously caused a beta-cutoff at the same ply.
 * During move ordering, killer moves are searched early to increase the chance
 * of early cutoffs when similar move patterns re-occur at the same ply.
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
 * The input moves are pseudo-legal; legality checks (e.g. leaving king in check)
 * are handled by the caller/search loop.
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
