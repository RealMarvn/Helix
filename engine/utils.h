//
// Created by Marvin Becker on 28.11.25.
//

#pragma once


/**
 * @brief Calculates the square in the board on the x and y coordinates.
 *
 * The position is calculated using the formula: ((y - 1) * 8 + x) - 1.
 * The grid has 8 columns and the indexing starts from 0, so the result is subtracted by 1.
 *
 * @param X The x-coordinate.
 * @param Y The y-coordinate.
 * @return The position in the grid.
 */
inline int calculateSquare(const int X, const int Y) { return (Y - 1) * 8 + X - 1; }
