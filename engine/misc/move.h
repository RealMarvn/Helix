//
// Created by Marvin Becker on 05.03.24.
//

#pragma once
#include <algorithm>
#include <array>
#include <cassert>
#include <sstream>
#include <string>

#include "./piece.h"

// Represents the maximum of moves someone could possibly do.
#define MAX_MOVES 218

// Normal can be seen as a capture or a move.
enum MoveType : uint8_t { NORMAL, EN_PASSANT, PROMOTION, CASTLING };

struct Move {
    // The square a piece want's to move to.
    int move_square{0};
    // The square the piece is on.
    int square{0};
    // The moving piece.
    Piece moving_piece;
    // The piece which gets captured (If it is a normal move it will be EMPTY)
    Piece captured_piece;
    // The piece a pawn can be promoted to. Can be empty.
    Piece promotion_piece;
    // The type of move.
    MoveType move_type{NORMAL};

    /**
     * @brief Converts the Move object's square and moveSquare coordinates to corresponding X and Y coordinates.
     *
     * This function takes the square and moveSquare coordinates of a Move object and converts them to X and Y
     * coordinates. The X coordinate is obtained by adding the ASCII value of 'a' to the remainder of square divided by 8,
     * and casting it to a character. The Y coordinate is obtained by adding 1 to the result of square divided by 8. The
     * same process is applied to the moveSquare coordinates to obtain the second set of X and Y coordinates. The
     * resulting X and Y coordinates are then concatenated together as a string and returned.
     *
     * @return A string representation of the X and Y coordinates of the Move object.
     */
    [[nodiscard]] std::string to_string() const {
        std::ostringstream out;
        out << static_cast<char>((square) % 8 + 'a') << (square) / 8 + 1;
        out << static_cast<char>((move_square) % 8 + 'a') << (move_square) / 8 + 1;
        if (move_type == PROMOTION) {
            out << static_cast<char>(std::tolower(promotion_piece.to_char()));
        }
        return out.str();
    }

    /**
     * @brief Overloaded equality operator for comparing two Move objects.
     *
     * @param other The other Move object to compare with.
     * @return True if the two Move objects are equal, false otherwise.
     */
    bool operator==(const Move& other) const {
        return move_square == other.move_square && square == other.square &&
               moving_piece.piece_type == other.moving_piece.piece_type &&
               captured_piece.piece_type == other.captured_piece.piece_type &&
               promotion_piece.piece_type == other.promotion_piece.piece_type && move_type == other.move_type;
    }
};

/**
 * @class PseudoLegalMoves
 *
 * @brief Represents a collection of pseudo-legal chess moves.
 *
 * The PseudoLegalMoves class provides a container for storing pseudo-legal chess moves. It uses a fixed-size array
 * to store the moves and provides methods for accessing and modifying the move list.
 */
class PseudoLegalMoves {
    // Core array which will hold the moves.
    std::array<Move, MAX_MOVES> move_list{};
    // The index of that array.
    int index = 0;

public:
    using iterator = std::array<Move, MAX_MOVES>::iterator;

    /**
     * @brief Returns an iterator pointing to the first element in the move list.
     *
     * This function returns an iterator pointing to the first element in the move list.
     *
     * @return An iterator pointing to the first element in the move list.
     */
    iterator begin() { return move_list.begin(); }

    /**
     * @fn iterator PseudoLegalMoves::end()
     *
     * @brief Returns an iterator one past the last element of the move list.
     *
     * This function returns an iterator pointing to one past the last element in the move list. It is used to indicate
     * the end of the range of elements in the move list.
     *
     * @return An iterator pointing to one past the last element of the move list.
     */
    iterator end() { return move_list.begin() + index; }

    /**
     * @brief Overloaded subscript operator for accessing a move in the move list.
     *
     * This function allows access to a move in the move list using the subscript operator [].
     * It performs index validation by asserting that the provided number is less than the index.
     * If the number is valid, the corresponding move is returned.
     *
     * @param number The index number of the desired move in the move list.
     * @return A reference to the move at the specified index.
     * @pre The move list must not be empty.
     * @pre Number must be less than the index.
     * @post None.
     */
    Move& operator[](const int number) {
        assert(number < index);
        return move_list[number];
    }

    /**
     * @brief Adds a move to the move list.
     *
     * This function adds a move to the move list by assigning it to the element at the current index. It then increments
     * the index by 1.
     *
     * @param mv The move to add to the move list.
     * @pre The index must be less than the maximum number of moves allowed.
     * @post The move is added to the move list and the index is incremented by 1.
     */
    void push_back(const Move& mv) {
        assert(index < MAX_MOVES);
        move_list[index] = mv;
        ++index;
    }

    /**
     * @fn bool PseudoLegalMoves::contains(const Move& mv)
     *
     * @brief Checks if the move list contains a specified move.
     *
     * This function checks if the move list contains the specified move. It uses the std::find algorithm to search for
     * the move in the move list. If the move is found, it returns true; otherwise, it returns false.
     *
     * @param mv The move to check for in the move list.
     * @return True if the move list contains the specified move, false otherwise.
     */
    bool contains(const Move& mv) { return std::find(begin(), end(), mv) != end(); }

    /**
     * @brief Sorts the move list in descending order based on mvv-lva.
     *
     * This function sorts the move list in descending order based on the score of each move.
     * It uses the std::sort algorithm with a lambda function as the comparison criterion.
     * The lambda function compares the scores of two moves and returns true if the score of the left move is greater than
     * the score of the right move. mvv-lva = most valuable victim, least valuable attacker
     *
     * @return None.
     */
    void sort_move_list_mvv_lva(const Move& ttMove) {
        std::sort(begin(), end(), [&](const Move& left, const Move& right) {
            return score_move(left, ttMove) > score_move(right, ttMove);
        });
    }

private:
    /**
     * @brief Calculates the score of a given move.
     *
     * This function calculates the score of a given move based on the piece types involved in the move.
     * The score is calculated by taking the piece type of the captured piece (modulo BP = 6 for no color piece)
     * and multiplying it by 10, and then subtracting the piece type of the moving piece (modulo BP = 6 for no color
     * piece).
     *
     * @param move The move for which to calculate the score.
     * @param ttMove The move from the transposition table
     * @return The score of the move.
     */
    static int score_move(const Move& move, const Move& ttMove) {
        if (move == ttMove) return 1000;
        if (move.captured_piece.piece_type == EMPTY) return 0;
        return move.captured_piece.piece_type % BP * 10 - move.moving_piece.piece_type % BP + 10;
    }
};
