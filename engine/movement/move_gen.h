//
// Created by Marvin Becker on 05.03.24.
//
/**
 * @file move_gen.h
 * @brief Declares move generation utilities for pseudo-legal chess moves.
 *
 * This header provides functions to generate pseudo-legal moves for each
 * piece type as well as a convenience function to gather all moves for a
 * given side on a Board. The functions operate on mailbox coordinates and
 * fill a PseudoLegalMoves container used later by the search.
 */

#pragma once
#include <array>
#include <utility>
#include <vector>

#include "../core/board.h"

/**
 * @namespace moveGenUtils
 * @brief Helper routines for generating pseudo-legal chess moves.
 *
 * Contains per-piece move generators and small lookup tables for pawn
 * promotions. All functions assume a consistent Board state and do not
 * perform legality checks such as leaving the king in check.
 */
namespace moveGenUtils {

    /**
     * @brief List of promotion piece types for white pawns.
     *
     * Ordered as queen, rook, knight, bishop to match typical
     * engine/GUI expectations when iterating promotion choices.
     */
    constexpr static std::array<PieceType, 4> white_pawn_possible_promotions = {WQ, WR, WN, WB};

    /**
     * @brief List of promotion piece types for black pawns.
     *
     * Mirrors white_pawn_possible_promotions but with black pieces.
     */
    constexpr static std::array<PieceType, 4> black_pawn_possible_promotions = {BQ, BR, BN, BB};

    /**
     * @brief Generates all pseudo-legal moves on the given board for a specific player.
     *
     * This function iterates through all the squares on the board and checks the piece on each square.
     * If the piece belongs to the specified player, it calls the appropriate helper function to generate all possible moves
     * for that piece. The generated moves are stored in the `allPseudoMoves` object.
     *
     * @param board The chess board.
     * @param PLAYER A boolean value indicating the player (true for white, false for black).
     * @return A `PseudoLegalMoves` object containing all the generated pseudo-legal moves.
     */
    PseudoLegalMoves get_all_pseudo_legal_moves(Board& board, bool PLAYER);

    /**
     * @brief Gets all possible moves for a rook piece on the given iterativeTimePoint square.
     *
     * This function calculates all possible moves for a rook located at the specified square on the chessboard.
     * It considers the current state of the board and stores all valid moves in the provided object.
     * The generated moves are pseudo-legal, meaning that they are not guaranteed to lead to a legal board position.
     *
     * @param start_square The iterativeTimePoint square of the rook piece.
     * @param board The Board object representing the current state of the chessboard.
     * @param all_pseudo_moves The PseudoLegalMoves object to add the possible rook moves to.
     * @param PIECE_COLOR The color of the rook piece (true for white, false for black).
     */
    void get_all_possible_rook_moves(std::pair<int, int> start_square, Board& board, PseudoLegalMoves& all_pseudo_moves,
                                     bool PIECE_COLOR);

    /**
     * \brief Generates all possible pawn moves from a given starting square.
     *
     * This function calculates all possible moves for a pawn located at the specified square on the chessboard.
     * It considers the current state of the board and stores all valid moves in the provided object.
     * The generated moves are pseudo-legal, meaning that they are not guaranteed to lead to a legal board position.
     *
     * \param START_SQUARE The starting square for generating moves.
     * \param board The chessboard.
     * \param all_pseudo_moves The container to store the generated moves.
     * \param PIECE_COLOR The color of the pawn (true for white, false for black).
     */
    void get_all_possible_pawn_moves(const std::pair<int, int>& START_SQUARE, Board& board,
                                     PseudoLegalMoves& all_pseudo_moves,
                                     bool PIECE_COLOR);

    /**
     * @brief Calculates all possible moves for a queen from a given iterativeTimePoint square.
     *
     * This function calculates all possible moves for a queen located at the specified square on the chessboard.
     * It considers the current state of the board and stores all valid moves in the provided object.
     * The generated moves are pseudo-legal, meaning that they are not guaranteed to lead to a legal board position.
     *
     * @param start_square The starting square of the queen.
     * @param board The chessboard object representing the current state of the game.
     * @param all_pseudo_moves A PseudoLegalMoves object to store all the possible moves.
     * @param PIECE_COLOR The color of the queen (true for white, false for black).
     */
    void get_all_possible_queen_moves(std::pair<int, int> start_square, Board& board,
                                      PseudoLegalMoves& all_pseudo_moves,
                                      bool PIECE_COLOR);

    /**
     * @brief Generates all possible moves for a king on the given board from the given iterativeTimePoint square.
     *
     * This function calculates all possible moves for a king located at the specified square on the chessboard.
     * It considers the current state of the board and stores all valid moves in the provided object.
     * The generated moves are pseudo-legal, meaning that they are not guaranteed to lead to a legal board position.
     *
     * @param START_SQUARE The iterativeTimePoint square of the king.
     * @param board The current board configuration.
     * @param all_pseudo_moves The PseudoLegalMoves object to store the generated moves.
     * @param PIECE_COLOR The color of the king's piece (true for white, false for black).
     *
     * @return None.
     */
    void get_all_possible_king_moves(const std::pair<int, int>& START_SQUARE, Board& board,
                                     PseudoLegalMoves& all_pseudo_moves,
                                     bool PIECE_COLOR);

    /**
     * @brief Get all possible knight moves from the given square.
     *
     * This function calculates all possible moves for a knight located at the specified square on the chessboard.
     * It considers the current state of the board and stores all valid moves in the provided object.
     * The generated moves are pseudo-legal, meaning that they are not guaranteed to lead to a legal board position.
     *
     * @param START_SQUARE The starting square for the knight.
     * @param board The chessboard.
     * @param all_pseudo_moves The PseudoLegalMoves object to store the possible moves.
     * @param PIECE_COLOR The color of the knight piece (true for white, false for black).
     */
    void get_all_possible_knight_moves(const std::pair<int, int>& START_SQUARE, Board& board,
                                       PseudoLegalMoves& all_pseudo_moves,
                                       bool PIECE_COLOR);

    /**
     * @brief Retrieves all possible moves for a bishop on the chessboard.
     *
     * This function calculates all possible moves for a bishop located at the specified square on the chessboard.
     * It considers the current state of the board and stores all valid moves in the provided object.
     * The generated moves are pseudo-legal, meaning that they are not guaranteed to lead to a legal board position.
     *
     * @param start_square The square where the bishop is located.
     * @param board The chessboard object representing the current state.
     * @param all_pseudo_moves The vector to store all possible moves.
     * @param PIECE_COLOR The color of the bishop (true for white, false for black).
     */
    void get_all_possible_bishop_moves(std::pair<int, int> start_square, Board& board,
                                       PseudoLegalMoves& all_pseudo_moves,
                                       bool PIECE_COLOR);
} // namespace moveGenUtils
