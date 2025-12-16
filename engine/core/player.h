//
// Created by Marvin Becker on 16.03.24.
//

/**
 * @file player.h
 * @brief Defines the player_type enumeration representing the side to move.
 *
 * This enum is used throughout the chess engine to indicate whether the
 * active player is White or Black. It is essential for move generation,
 * evaluation, and search control flow.
 */

#pragma once

/**
 * @brief Enumeration representing which player is to move.
 *
 * WHITE — White is the active player
 * BLACK — Black is the active player
 *
 * Used within Board, MoveGenerator, Search, and UCI modules.
 */
enum player_type : uint8_t { WHITE, BLACK };
