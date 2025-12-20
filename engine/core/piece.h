//
// Created by Marvin Becker on 05.03.24.
//

/**
 * @file piece.h
 * @brief Defines PieceType and the Piece class used to represent chess pieces.
 *
 * This file provides an enumeration of all colored piece types and the Piece
 * class, which encapsulates piece identity, material values, game-phase values,
 * character conversion, and utility functions used throughout the engine.
 */

#pragma once

/**
 * @brief Enumeration of all chess piece types including color.
 *
 * White pieces are listed first (WP..WK), followed by black pieces (BP..BK),
 * and finally EMPTY. The numeric ordering allows color checks using simple
 * comparisons (e.g., piece_type < BP means white).
 */
// Represents the pieces with colors. Color + Piece = White + Pawn = WP
enum PieceType : uint8_t { WP, WN, WB, WR, WQ, WK, BP, BN, BB, BR, BQ, BK, EMPTY };

/**
 * @brief The Piece class represents a chess piece.
 *
 * This class encapsulates the data and behavior of a chess piece.
 * It has a pieceType member variable which stores the type of the piece.
 * The class provides methods to retrieve information about the piece, such as whether it is white,
 * its character representation, and its material value.
 * It also provides static member variables and methods for piece value and character mappings.
 */
class Piece {
public:

    /**
     * @brief Constructs a new Piece object with the given piece type.
     *
     * @param piece The PieceType representing the piece type.
     */
    explicit Piece(const PieceType piece) : piece_type_{piece} {
    }

    /**
     * @brief Constructs a new Piece object from a character representation.
     *
     * @param piece The character representing the piece type.
     */
    explicit Piece(const char piece) : piece_type_{find_key_by_value(piece)} {
    };

    /**
     * @brief Constructs an EMPTY piece.
     *
     * Used when initializing empty squares on the board or representing
     * an absence of a piece.
     */
    Piece() = default;

    /**
     * @brief The underlying piece type stored by this Piece instance.
     *
     * Defaults to EMPTY when using the default constructor.
     */
    PieceType piece_type_{EMPTY};

    /**
     * @brief Checks if the piece is white.
     *
     * This function returns true if the piece is white, and false otherwise.
     *
     * @return True if the piece is white, false otherwise.
     */
    [[nodiscard]] bool is_white() const { return (piece_type_ < BP); }

    /**
     * @brief Returns the character representation of the piece.
     *
     * This function returns the character representation of the piece based on its pieceType.
     *
     * @return The character representation of the piece.
     */
    [[nodiscard]] char to_char() const { return PIECE_TO_CHAR_[piece_type_]; }

    /**
     * @brief Retrieves the material value of the piece.
     *
     * This method calculates and returns the material value of the piece.
     * The material value is based on the type of the piece and whether it is white.
     * If the piece type is EMPTY, the material value is 0.
     * If the piece type is not EMPTY, the material value is calculated as follows:
     * - If the piece is white, the material value is positive.
     * - If the piece is black, the material value is negative.
     * - The material value is multiplied by the mg_pieceValue of the piece type.
     *
     * @return The material value of the piece.
     */
    [[nodiscard]] int get_material_value(const bool end_game) const {
        if (piece_type_ == EMPTY) return 0;
        return end_game ? EG_PIECE_VALUE_[piece_type_ % BP] : MG_PIECE_VALUE[piece_type_ % BP];
    }

    /**
     * @brief Retrieves the game phase value of the piece.
     *
     * This method retrieves the game phase value of the piece based on its pieceType.
     * The game phase value represents the piece's importance of the game.
     * It is used to calculate the current game phase.
     * To learn more about the game phase value check out 'Tampered Eval'.
     *
     * If the piece type is EMPTY, the game phase value is 0.
     * If the piece is not EMPTY, the game phase value is retrieved from the gamePhaseValue array
     * based on the piece type modulo BP (i.e., the piece type index in the array).
     *
     * @return The game phase value of the piece.
     */
    [[nodiscard]] int get_game_phase_value() const {
        if (piece_type_ == EMPTY) return 0;
        return GAME_PHASE_VALUE_[piece_type_ % BP];
    }

private:

    /**
     * @brief Game‑phase contribution values for each piece type.
     *
     * Used to calculate the overall game phase (opening → endgame)
     * following PeSTO-style evaluation.
     */
    constexpr static int GAME_PHASE_VALUE_[6] = {0, 1, 1, 2, 4, 0};

    /**
     * @brief Endgame material values indexed by base piece type (pawn..king).
     *
     * These values are applied when the engine detects an endgame phase.
     */
    constexpr static int EG_PIECE_VALUE_[6] = {94, 281, 297, 512, 936, 0};

    /**
     * @brief Midgame material values indexed by base piece type (pawn..king).
     *
     * Used for evaluating positions during the midgame phase.
     */
    constexpr static int MG_PIECE_VALUE[6] = {82, 337, 365, 477, 1025, 0};

    /**
     * @brief Mapping from PieceType to corresponding FEN character.
     *
     * Uppercase = white pieces, lowercase = black pieces, ' ' = EMPTY.
     */
    constexpr static char PIECE_TO_CHAR_[13] = {'P', 'N', 'B', 'R', 'Q', 'K', 'p', 'n', 'b', 'r', 'q', 'k', ' '};

    /**
     * @brief Finds the PieceType based on the given character value.
     *
     * This function searches for the character value in the pieceToChar array
     * and returns the corresponding PieceType.
     * If the character value is not found in the array, it returns EMPTY.
     *
     * @param value The character value representing the piece.
     * @return The PieceType corresponding to the character value.
     */
    static PieceType find_key_by_value(const char value) {
        // Go through the array and find the matching piece.
        for (int i = 0; i < 13; ++i) {
            if (PIECE_TO_CHAR_[i] == value) return static_cast<PieceType>(i);
        }
        return EMPTY;
    }
};
