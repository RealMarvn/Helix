//
// Created by Marvin Becker on 28.11.25.
//

/**
 * @file utils.h
 * @brief General utility helpers used across the chess engine.
 *
 * Currently contains coordinate conversion helpers used for mapping
 * human-readable (x, y) board coordinates into mailbox indices.
 */

#pragma once


/**
 * @brief Converts 1-based board coordinates into a 0-based mailbox index.
 *
 * The mapping follows:
 *   - Files (columns) = X = 1..8
 *   - Ranks (rows)    = Y = 1..8
 *
 * Formula: ((Y - 1) * 8 + X) - 1
 *
 * Example:
 *   X=1, Y=1 → square 0 (a1)
 *   X=8, Y=8 → square 63 (h8)
 *
 * @param X File (column) in range [1, 8].
 * @param Y Rank (row) in range [1, 8].
 * @return Mailbox index in [0, 63].
 */
inline int calculateSquare(const int X, const int Y) { return (Y - 1) * 8 + X - 1; }
