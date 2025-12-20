//
// Created by Marvin Becker on 20.12.25.
//

#include "debug_print.h"

#include "./core/board.h"
#include "./core/move.h"
#include "./movement/move_gen.h"
#include "heuristics.h"
#include "search.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

namespace search::debug
{

void print_health(const ChessBot& bot)
{
    const auto NODES = bot.nodes;
    const auto QNODES = bot.qnodes;
    const double Q_RATIO =
        (NODES > 0) ? static_cast<double>(QNODES) / static_cast<double>(NODES) : 0.0;

    std::cout << "info string HEALTH"
              << " qnodes=" << QNODES << " qratio=" << Q_RATIO << std::endl;
}

void print_tt(const ChessBot& bot)
{
    const auto& [PROBES, HITS, STORES, REPLACES] = bot.tt.get_stats();

    std::cout << "info string TT"
              << " probes=" << PROBES << " hits=" << HITS << " stores=" << STORES
              << " replaces=" << REPLACES << " returns=" << bot.tt_returns << std::endl;
}

void print_root_ordering(const ChessBot& bot, Board& board)
{
    Move tt_move{};
    bot.tt.probe_move(board.get_hash(), tt_move);

    // Get all moves
    auto moves = moveGenUtils::get_all_pseudo_legal_moves(board, board.player_ == WHITE);

    // Order them with PLY 0
    search::heuristics::order_moves(moves, tt_move, 0, board.player_, bot.killers, bot.history);

    constexpr int TOP_N = 6;
    const int n = std::min(TOP_N, moves.size());

    std::cout << "info string ORD"
              << " tt=" << (tt_move.is_null() ? std::string("none") : tt_move.to_string())
              << " top=[";

    for (int i = 0; i < n; ++i)
    {
        if (i)
            std::cout << ',';
        std::cout << moves[i].to_string();
    }

    std::cout << "]" << std::endl;
}

void print_pv(const ChessBot& bot, const Board& board)
{
    Board b = board;

    constexpr int MAX_PLIES = 10;
    std::vector<std::string> pv;
    pv.reserve(MAX_PLIES);

    std::unordered_set<std::uint64_t> seen;
    seen.reserve(MAX_PLIES + 1);

    for (int ply = 0; ply < MAX_PLIES; ++ply)
    {
        const auto key = b.get_hash();
        if (!seen.insert(key).second)
            break;

        Move m{};
        if (!bot.tt.probe_move(key, m) || m.is_null())
            break;

        if (!b.make_move(m))
            break;

        pv.push_back(m.to_string());
    }

    std::cout << "info string PV pv=[";
    for (std::size_t i = 0; i < pv.size(); ++i)
    {
        if (i)
            std::cout << ',';
        std::cout << pv[i];
    }
    std::cout << "]" << std::endl;
}

} // namespace search::debug