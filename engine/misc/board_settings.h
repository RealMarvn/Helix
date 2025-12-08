//
// Created by Marvin Becker on 14.03.24.
//

#pragma once

struct board_setting {
    // EP square
    int ep_square{100};
    // Castling moves.
    bool white_queen_side{false};
    bool white_king_side{false};
    bool black_queen_side{false};
    bool black_king_side{false};

    // For FEN
    int last_moves_since_pawn_or_capture{0};
    int turns{1};
};
