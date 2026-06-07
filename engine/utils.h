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

#include <string>

/**
 * @brief Converts 1-based board coordinates (file, rank) into a 0-based mailbox index.
 *
 * Coordinate conventions:
 *  - X (file) in [1..8] corresponds to a..h
 *  - Y (rank) in [1..8] corresponds to 1..8
 *
 * Mailbox index conventions (0-based):
 *  - a1 -> 0, h1 -> 7
 *  - a8 -> 56, h8 -> 63
 *
 * @param X File (column) in range [1, 8].
 * @param Y Rank (row) in range [1, 8].
 * @return Mailbox index in [0, 63].
 *
 * @note No bounds checking is performed. Validate X/Y before calling if input is external.
 */
inline int calculateSquare(const int X, const int Y) { return (Y - 1) * 8 + X - 1; }

/**
 * @brief Checks whether a string starts with the given C-string prefix.
 *
 * Used primarily for lightweight command parsing (e.g. UCI / CLI), where we
 * frequently need to match known command prefixes.
 *
 * @param s       Input string.
 * @param prefix  Null-terminated prefix string.
 * @return true if @p s begins with @p prefix, otherwise false.
 */
inline bool starts_with(const std::string& s, const char* prefix)
{
    return s.rfind(prefix, 0) == 0;
}