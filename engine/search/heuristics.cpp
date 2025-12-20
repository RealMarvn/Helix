
//
// Created by Marvin Becker on 16.12.25.
//

#include "heuristics.h"

#include <algorithm>

static int score_move_heuristic(const Move& move, const Move& tt_move, const int ply,
                                const int side, const search::heuristics::KillerTable& killers,
                                const search::heuristics::HistoryTable& history)
{
    // TT / PV move always first.
    if (tt_move.square_ != tt_move.move_square_ && (move == tt_move))
        return search::heuristics::HEUR_TT_BONUS;

    // Captures before quiets (MVV-LVA)
    if (move.captured_piece_.piece_type_ != EMPTY || move.move_type_ == EN_PASSANT)
    {
        // For en-passant, the captured pawn is not on the target square, so captured_piece can be
        // EMPTY. Still treat it as a pawn victim for ordering.
        const int victim = (move.move_type_ == EN_PASSANT)
                               ? 0 // pawn
                               : (move.captured_piece_.piece_type_ % BP);

        const int attacker = (move.moving_piece_.piece_type_ % BP);
        return 10000 + victim * 100 - attacker; // base so captures stay above most quiets
    }

    // Quiet move heuristics.
    int score = 0;
    if (killers.is_killer1(ply, move))
        score += search::heuristics::HEUR_KILLER1_BONUS;

    else if (killers.is_killer2(ply, move))
        score += search::heuristics::HEUR_KILLER2_BONUS;

    // History is additive and smaller than killer bonuses.
    score += history.get(side, move.square_, move.move_square_);
    return score;
}

namespace search::heuristics
{

void KillerTable::clear()
{
    for (auto& k : killers_)
    {
        k[0] = Move{};
        k[1] = Move{};
    }
}

void KillerTable::add(const int ply, const Move& move)
{
    if (ply < 0 || ply >= HEUR_MAX_PLY)
        return;

    // Avoid duplicates
    if (killers_[ply][0] == move)
        return;

    killers_[ply][1] = killers_[ply][0];
    killers_[ply][0] = move;
}

bool KillerTable::is_killer1(const int ply, const Move& move) const
{
    if (ply < 0 || ply >= HEUR_MAX_PLY)
        return false;
    return killers_[ply][0] == move;
}

bool KillerTable::is_killer2(const int ply, const Move& move) const
{
    if (ply < 0 || ply >= HEUR_MAX_PLY)
        return false;
    return killers_[ply][1] == move;
}

void HistoryTable::clear()
{
    for (auto& side : history_)
        for (auto& from : side)
            from.fill(0);
}

void HistoryTable::add(const int side, const int from, const int to, const int depth)
{
    if (side < 0 || side > 1)
        return;
    if (from < 0 || from >= 64 || to < 0 || to >= 64)
        return;
    if (depth <= 0)
        return;

    const int bonus = depth * depth;
    int& cell = history_[side][from][to];

    // Saturating add (avoid runaway/overflow)
    if (constexpr int MAX_H = 1000000; cell > MAX_H - bonus)
        cell = MAX_H;
    else
        cell += bonus;
}

int HistoryTable::get(const int side, const int from, const int to) const
{
    if (side < 0 || side > 1)
        return 0;
    if (from < 0 || from >= 64 || to < 0 || to >= 64)
        return 0;
    return history_[side][from][to];
}

void order_moves(PseudoLegalMoves& moves, const Move& tt_move, const int ply, const int side,
                 const KillerTable& killers, const HistoryTable& history)
{
    std::sort(moves.begin(), moves.end(), [&](const Move& a, const Move& b) {
        const int sa = score_move_heuristic(a, tt_move, ply, side, killers, history);
        const int sb = score_move_heuristic(b, tt_move, ply, side, killers, history);
        return sa > sb;
    });
}

} // namespace search::heuristics
