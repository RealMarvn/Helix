//
// Created by Marvin Becker on 20.12.25.
//

#pragma once

#include <cstdint>

// Forward declarations to avoid heavy includes in headers
class Board;
class ChessBot;

namespace search::debug
{
    // High-level entry points used by ChessBot::print_debug
    void print_health(const ChessBot& bot);
    void print_tt(const ChessBot& bot);

    // VERBOSE helpers (root-only)
    void print_root_ordering(const ChessBot& bot, Board& board);
    void print_pv(const ChessBot& bot, const Board& board);
}


