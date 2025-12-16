
//
// Created by Marvin Becker on 16.12.25.
//

#include "heuristics.h"

#include <algorithm>

void search::heuristics::KillerTable::clear()
{
    for (auto& k : killers)
    {
        k[0] = Move{};
        k[1] = Move{};
    }
}

void search::heuristics::KillerTable::add(const int PLY, const Move& m)
{
    if (PLY < 0 || PLY >= HEUR_MAX_PLY)
        return;

    // Avoid duplicates
    if (killers[PLY][0] == m)
        return;

    killers[PLY][1] = killers[PLY][0];
    killers[PLY][0] = m;
}

bool search::heuristics::KillerTable::is_killer1(const int PLY, const Move& m) const
{
    if (PLY < 0 || PLY >= HEUR_MAX_PLY)
        return false;
    return killers[PLY][0] == m;
}

bool search::heuristics::KillerTable::is_killer2(const int PLY, const Move& m) const
{
    if (PLY < 0 || PLY >= HEUR_MAX_PLY)
        return false;
    return killers[PLY][1] == m;
}

void search::heuristics::HistoryTable::clear()
{
    for (auto& side : h)
        for (auto& from : side)
            from.fill(0);
}

void search::heuristics::HistoryTable::add(const int SIDE, const int FROM, const int TO,
                                           const int DEPTH)
{
    if (SIDE < 0 || SIDE > 1)
        return;
    if (FROM < 0 || FROM >= 64 || TO < 0 || TO >= 64)
        return;
    if (DEPTH <= 0)
        return;

    const int bonus = DEPTH * DEPTH;
    int& cell = h[SIDE][FROM][TO];

    // Saturating add (avoid runaway/overflow)
    if (constexpr int MAX_H = 1'000'000; cell > MAX_H - bonus)
        cell = MAX_H;
    else
        cell += bonus;
}

int search::heuristics::HistoryTable::get(const int SIDE, const int FROM, const int TO) const
{
    if (SIDE < 0 || SIDE > 1)
        return 0;
    if (FROM < 0 || FROM >= 64 || TO < 0 || TO >= 64)
        return 0;
    return h[SIDE][FROM][TO];
}

static int score_move_heuristic(const Move& M, const Move& TT_MOVE, const int PLY, const int SIDE,
                                const search::heuristics::KillerTable& KILLERS,
                                const search::heuristics::HistoryTable& HISTORY)
{
    // TT / PV move always first.
    if (TT_MOVE.square != TT_MOVE.move_square && (M == TT_MOVE))
        return search::heuristics::HEUR_TT_BONUS;

    // Captures before quiets (MVV-LVA)
    if (M.captured_piece.piece_type != EMPTY || M.move_type == EN_PASSANT)
    {
        // For en-passant, the captured pawn is not on the target square, so captured_piece can be
        // EMPTY. Still treat it as a pawn victim for ordering.
        const int victim = (M.move_type == EN_PASSANT) ? 0 // pawn
                                                       : (M.captured_piece.piece_type % BP);

        const int attacker = (M.moving_piece.piece_type % BP);
        return 10000 + victim * 100 - attacker; // base so captures stay above most quiets
    }

    // Quiet move heuristics.
    int score = 0;
    if (KILLERS.is_killer1(PLY, M))
        score += search::heuristics::HEUR_KILLER1_BONUS;
    else if (KILLERS.is_killer2(PLY, M))
        score += search::heuristics::HEUR_KILLER2_BONUS;

    // History is additive and smaller than killer bonuses.
    score += HISTORY.get(SIDE, M.square, M.move_square);
    return score;
}

void search::heuristics::order_moves(PseudoLegalMoves& moves, const Move& tt_move, const int ply,
                                     const int side, const search::heuristics::KillerTable& killers,
                                     const search::heuristics::HistoryTable& history)
{
    std::sort(moves.begin(), moves.end(), [&](const Move& a, const Move& b) {
        const int sa = score_move_heuristic(a, tt_move, ply, side, killers, history);
        const int sb = score_move_heuristic(b, tt_move, ply, side, killers, history);
        return sa > sb;
    });
}
