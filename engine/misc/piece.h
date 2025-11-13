//
// Created by Marvin Becker on 05.03.24.
//

#pragma once

// Represents the pieces with colors. Color + Piece = White + Pawn = WP
enum PieceType { WP, WN, WB, WR, WQ, WK, BP, BN, BB, BR, BQ, BK, EMPTY };

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
    explicit Piece(PieceType piece) : pieceType{piece} {
    };

    /**
     * @brief Constructs a new Piece object with the given piece type.
     *
     * @param piece The character representing the piece type.
     */
    explicit Piece(char piece) : pieceType{findKeyByValue(piece)} {
    };

    /**
     * @class Piece
     * Represents an EMPTY piece.
     */
    Piece() : pieceType{EMPTY} {
    };

    PieceType pieceType{EMPTY};

    /**
     * @brief Checks if the piece is white.
     *
     * This function returns true if the piece is white, and false otherwise.
     *
     * @return True if the piece is white, false otherwise.
     */
    [[nodiscard]] inline bool isWhite() const { return (pieceType < BP); }

    /**
     * @brief Returns the character representation of the piece.
     *
     * This function returns the character representation of the piece based on its pieceType.
     *
     * @return The character representation of the piece.
     */
    [[nodiscard]] inline char toChar() const { return pieceToChar[pieceType]; }

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
    [[nodiscard]] int getMaterialValue(bool endGame) const {
        if (pieceType == EMPTY) return 0;
        return ((endGame ? eg_pieceValue[pieceType % BP] : mg_pieceValue[pieceType % BP]));
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
    [[nodiscard]] int getGamePhaseValue() const {
        if (pieceType == EMPTY) return 0;
        return gamePhaseValue[pieceType % BP];
    }

private:
    // Values from (https://www.chessprogramming.org/PeSTO's_Evaluation_Function)
    constexpr static int gamePhaseValue[6] = {0, 1, 1, 2, 4, 0};
    // End game value for BP, BK, BB, BR, BQ and BK
    constexpr static int eg_pieceValue[6] = {94, 281, 297, 512, 936, 0};
    // Mid game value for BP, BK, BB, BR, BQ and BK
    constexpr static int mg_pieceValue[6] = {82, 337, 365, 477, 1025, 0};
    // Array to get the chars.
    constexpr static char pieceToChar[13] = {'P', 'N', 'B', 'R', 'Q', 'K', 'p', 'n', 'b', 'r', 'q', 'k', ' '};

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
    static PieceType findKeyByValue(char value) {
        // Go through the array and find the matching piece.
        for (int i = 0; i < 13; ++i) {
            if (pieceToChar[i] == value) return static_cast<PieceType>(i);
        }
        return EMPTY;
    }
};
