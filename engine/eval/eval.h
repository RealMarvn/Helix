//
// Created by Marvin Becker on 16.12.25.
//

#pragma once

#include <cstdint>

// Forward declaration to avoid pulling heavy headers into eval users.
// Include the real Board header in eval.cpp.
class Board;

/**
 * @brief Static evaluation of chess positions.
 *
 * The evaluation is independent of the search (Negamax/AlphaBeta/TT) and must not mutate the board.
 *
 * Convention:
 * - Positive score means the side to move is better.
 * - Negative score means the side to move is worse.
 *
 * Units are engine-internal (typically centipawns, but any consistent scale is fine).
 */
namespace eval
{
    /**
     * @brief Evaluate a position.
     *
     * @param board Current position.
     * @return Signed evaluation from the perspective of the side to move.
     */
    [[nodiscard]] int evaluate(const Board& board);
}
