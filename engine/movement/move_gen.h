//
// Created by Marvin Becker on 05.03.24.
//

#pragma once
#include <array>
#include <utility>
#include <vector>

#include "../board.h"

namespace moveGenUtils {
    static constexpr std::array<PieceType, 4> whitePawnPossiblePromotions = {WQ, WR, WN, WB};
    static constexpr std::array<PieceType, 4> blackPawnPossiblePromotions = {BQ, BR, BN, BB};

    /**
     * @brief Generates all pseudo-legal moves on the given board for a specific player.
     *
     * This function iterates through all the squares on the board and checks the piece on each square.
     * If the piece belongs to the specified player, it calls the appropriate helper function to generate all possible moves
     * for that piece. The generated moves are stored in the `allPseudoMoves` object.
     *
     * @param board The chess board.
     * @param player A boolean value indicating the player (true for white, false for black).
     * @return A `PseudoLegalMoves` object containing all the generated pseudo-legal moves.
     */
    PseudoLegalMoves getAllPseudoLegalMoves(Board& board, bool player);

    /**
     * @brief Gets all possible moves for a rook piece on the given iterativeTimePoint square.
     *
     * This function calculates all possible moves for a rook located at the specified square on the chessboard.
     * It considers the current state of the board and stores all valid moves in the provided object.
     * The generated moves are pseudo-legal, meaning that they are not guaranteed to lead to a legal board position.
     *
     * @param startSquare The iterativeTimePoint square of the rook piece.
     * @param board The Board object representing the current state of the chessboard.
     * @param allPseudoMoves The PseudoLegalMoves object to add the possible rook moves to.
     * @param pieceColor The color of the rook piece (true for white, false for black).
     */
    void getAllPossibleRookMoves(std::pair<int, int> startSquare, Board& board, PseudoLegalMoves& allPseudoMoves,
                                 bool pieceColor);

    /**
     * \brief Generates all possible pawn moves from a given starting square.
     *
     * This function calculates all possible moves for a pawn located at the specified square on the chessboard.
     * It considers the current state of the board and stores all valid moves in the provided object.
     * The generated moves are pseudo-legal, meaning that they are not guaranteed to lead to a legal board position.
     *
     * \param startSquare The starting square for generating moves.
     * \param board The chessboard.
     * \param allPseudoMoves The container to store the generated moves.
     * \param pieceColor The color of the pawn (true for white, false for black).
     */
    void getAllPossiblePawnMoves(const std::pair<int, int>& startSquare, Board& board, PseudoLegalMoves& allPseudoMoves,
                                 bool pieceColor);

    /**
     * @brief Calculates all possible moves for a queen from a given iterativeTimePoint square.
     *
     * This function calculates all possible moves for a queen located at the specified square on the chessboard.
     * It considers the current state of the board and stores all valid moves in the provided object.
     * The generated moves are pseudo-legal, meaning that they are not guaranteed to lead to a legal board position.
     *
     * @param startSquare The starting square of the queen.
     * @param board The chessboard object representing the current state of the game.
     * @param allPseudoMoves A PseudoLegalMoves object to store all the possible moves.
     * @param pieceColor The color of the queen (true for white, false for black).
     */
    void getAllPossibleQueenMoves(std::pair<int, int> startSquare, Board& board, PseudoLegalMoves& allPseudoMoves,
                                  bool pieceColor);

    /**
     * @brief Generates all possible moves for a king on the given board from the given iterativeTimePoint square.
     *
     * This function calculates all possible moves for a king located at the specified square on the chessboard.
     * It considers the current state of the board and stores all valid moves in the provided object.
     * The generated moves are pseudo-legal, meaning that they are not guaranteed to lead to a legal board position.
     *
     * @param startSquare The iterativeTimePoint square of the king.
     * @param board The current board configuration.
     * @param allPseudoMoves The PseudoLegalMoves object to store the generated moves.
     * @param pieceColor The color of the king's piece (true for white, false for black).
     *
     * @return None.
     */
    void getAllPossibleKingMoves(const std::pair<int, int>& startSquare, Board& board, PseudoLegalMoves& allPseudoMoves,
                                 bool pieceColor);

    /**
     * @brief Get all possible knight moves from the given square.
     *
     * This function calculates all possible moves for a knight located at the specified square on the chessboard.
     * It considers the current state of the board and stores all valid moves in the provided object.
     * The generated moves are pseudo-legal, meaning that they are not guaranteed to lead to a legal board position.
     *
     * @param startSquare The starting square for the knight.
     * @param board The chessboard.
     * @param allPseudoMoves The PseudoLegalMoves object to store the possible moves.
     * @param pieceColor The color of the knight piece (true for white, false for black).
     */
    void getAllPossibleKnightMoves(const std::pair<int, int>& startSquare, Board& board,
                                   PseudoLegalMoves& allPseudoMoves,
                                   bool pieceColor);

    /**
     * @brief Retrieves all possible moves for a bishop on the chessboard.
     *
     * This function calculates all possible moves for a bishop located at the specified square on the chessboard.
     * It considers the current state of the board and stores all valid moves in the provided object.
     * The generated moves are pseudo-legal, meaning that they are not guaranteed to lead to a legal board position.
     *
     * @param startSquare The square where the bishop is located.
     * @param board The chessboard object representing the current state.
     * @param allPseudoMoves The vector to store all possible moves.
     * @param pieceColor The color of the bishop (true for white, false for black).
     */
    void getAllPossibleBishopMoves(std::pair<int, int> startSquare, Board& board, PseudoLegalMoves& allPseudoMoves,
                                   bool pieceColor);
} // namespace moveGenUtils
