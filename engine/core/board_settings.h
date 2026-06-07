//
// Created by Marvin Becker on 14.03.24.
//

/**
 * @file board_settings.h
 * @brief Stores auxiliary state information used by the chess board.
 *
 * This structure encapsulates metadata not directly contained in the
 * piece representation, including castling rights, en-passant state.
 */

#pragma once

/**
 * @brief Represents auxiliary board state required for move generation
 * and FEN compatibility.
 *
 * This includes castling permissions, en-passant availability.
 */
struct board_setting {

    /**
     * @brief En-passant target square.
     *
     * Contains a mailbox index representing the square where an
     * en-passant capture is possible. A value of 100 means no EP square.
     */
    int ep_square_{100};

    /**
     * @brief Castling availability flags.
     *
     * These four booleans indicate whether each side still retains
     * its respective castling rights.
     */
    bool white_queen_side_{false};
    bool white_king_side_{false};
    bool black_queen_side_{false};
    bool black_king_side_{false};

    /**
     * @brief Half-move counter for the fifty-move rule.
     *
     * Increments after every move that is neither a pawn move nor a capture.
     */
    int last_moves_since_pawn_or_capture_{0};

    /**
     * @brief Full-move counter used in FEN.
     *
     * Starts at 1 and increments after Black's move, as defined by PGN/FEN.
     */
    int turns_{1};
};
