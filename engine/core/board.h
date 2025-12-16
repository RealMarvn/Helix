//
// Created by Marvin Becker on 15.12.23.
//

/**
 * @file board.h
 * @brief Defines the Board class representing a chess position.
 *
 * The Board class stores piece placement, side to move, auxiliary state
 * (castling, en-passant, move counters) and the Zobrist hash. It provides
 * helper functions for move making/unmaking, FEN I/O, check detection and
 * basic game-state queries used by the search.
 */

#pragma once

#include <cassert>
#include <map>
#include <random>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "board_settings.h"
#include "move.h"
#include "piece.h"
#include "player.h"
#include "zobrist.h"

/**
 * @class Board
 * @brief Represents a full chess position including side to move and metadata.
 *
 * The Board class owns the 64-square mailbox representation, the side to move,
 * a history of moves and board settings, and a Zobrist hash of the current
 * state. It offers functionality for move generation clients such as
 * making/unmaking moves, checking for check/checkmate, and converting to/from
 * FEN.
 */
class Board {
public:

    /**
     * @brief Side to move on the current board.
     *
     * WHITE or BLACK depending on whose turn it is.
     */
    player_type player{WHITE};

    /**
     * @brief History of all moves that have been played on this board.
     *
     * Used mainly for undoing moves and for debugging/printing purposes.
     */
    std::vector<Move> moves;

    /**
     * @brief History of board settings corresponding to each move.
     *
     * Stores auxiliary state (castling rights, en-passant, counters) for
     * each ply to support precise undo operations.
     */
    std::vector<board_setting> history;

    /**
     * @brief Current auxiliary board settings.
     *
     * Contains castling rights, en-passant square and move counters as in FEN.
     */
    board_setting board_settings;

    /**
     * @brief Direct access to a square of the internal board array.
     *
     * @param INDEX Mailbox index in [0, 63].
     * @return Reference to the Piece on that square.
     */
    Piece& operator[](const int INDEX) {
        assert(INDEX < 64 && INDEX >= 0);
        return board[INDEX];
    }

    const Piece& operator[](const int INDEX) const {
        assert(INDEX < 64 && INDEX >= 0);
        return board[INDEX];
    }

    /**
     * @brief Initializes a new Board object.
     *
     * This constructor initializes a new Board object. It sets the current player to WHITE and initializes
     * each piece on the board to an empty piece. It then reads the initial board state from a FEN string.
     */
    Board() {
        read_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    }

    /**
     * @brief Checks if the king of the given piece color is in check.
     *
     * This function iterates through the board to find the king of the given piece color.
     * Once the king is found, it calls the isSquareAttacked function to check if the king is being attacked.
     *
     * @param PIECE_COLOR The color of the king to check (true for white, false for black).
     * @return True if the king is in check, false otherwise.
     */
    bool is_king_in_check(bool PIECE_COLOR);

    /**
     * @brief Resets the board to the starting position;
     *
     * This function checks reloads the default starting position and clears the movement cache.
     *
     */
    void reset();

    /**
     * @brief Checks if the given square is attacked by any piece of the specified opponents color.
     *
     * This function checks if the given square on the chessboard is attacked by any piece of the specified side to moves
     * counterpart (super piece method). It iterates through all possible knight, pawn, bishop, and rook moves and checks
     * if any of them captures their counterpart on the given square. It also checks if the square is attacked by the king
     * in the adjacent positions.
     *
     * @param SQUARE The square to check.
     * @param PIECE_COLOR The color of the piece to check (true for white, false for black).
     * @return True if the square is attacked, false otherwise.
     */
    bool is_square_attacked(const std::pair<int, int>& SQUARE, bool PIECE_COLOR);

    /**
     * @brief Attempts to move a chess piece on the board.
     *
     * This function attempts to move a chess piece on the board. It checks various conditions such as legality of the
     * move and checks for pieces belonging to the opponent. If the move is successful, it updates the board state
     * accordingly.
     *
     * @param MOVE The move to be made.
     * @return True if the move is successful, false otherwise. If false, the move will not be applied.
     */
    bool try_to_move_piece(const Move& MOVE);

    /**
     * @brief Moves a chess piece on the board.
     *
     * This function moves a chess piece on the board. It only checks if the move is legal (you don't set yourself in
     * check) and updates the board accordingly. This function does not check if the move is correct. Use @see
     * tryToMovePiece for correct checking. The function returns true if the move is successful, and false otherwise. If
     * false the move will not be applied!
     *
     * @param MOVE The move to make.
     * @return True if the move is successful, false otherwise. If false the move will not be applied!.
     */
    bool make_move(const Move& MOVE);

    /**
     * @brief Removes the last move from the list of moves and updates the board state accordingly.
     *
     * This function removes the last move from the list of moves and updates the board state by reversing the changes
     * made by the move. It returns true if the move was successfully undone, and false otherwise.
     *
     * @return True if the last move was successfully undone, false otherwise.
     */
    bool pop_last_move();

    /**
     * @brief Prints the current state of the chessboard.
     *
     * This function prints the current state of the chessboard.
     * It shows the current turn and the positions of all the pieces on the board.
     */
    void print_current_board() const;

    /**
     * @brief Reads a FEN string and sets up the board accordingly.
     *
     * This function reads a FEN (Forsyth-Edwards Notation) string and sets up the chessboard based on the provided FEN
     * string. Then FEN has to be correct. Otherwise the programm will throw an exception!
     *
     * @param INPUT The FEN string to parse.
     */
    void read_fen(const std::string& INPUT);

    /**
     * @brief Returns the current FEN representation of the chessboard.
     *
     * This function returns the current FEN (Forsyth-Edwards Notation) representation of the chessboard.
     * The FEN string represents the board state, the current player to move, castling rights and the en passant target
     * square.
     *
     * @return The current FEN representation of the chessboard.
     */
    [[nodiscard]] std::string get_fen() const;

    /**
     * @brief Checks if the current player is in checkmate.
     *
     * This function determines if the current player is in checkmate.
     * It generates all possible pseudo-legal moves for the current player and checks if any of them result in a
     * non-checkmate position. If there are no such moves, then checkmate is detected.
     *
     * @param isWhite True if the current player is white, false otherwise.
     * @return True if the current player is in checkmate, false otherwise.
     */
    bool is_check_mate(bool isWhite);

    /**
     * @brief Parses a chess move from a string input.
     *
     * This function takes a string input representing a chess move and parses it into a Move object.
     * The string input should follow this notation: Pa2(x)a3(=Q).
     * First the piece, second the square, third capture is optional, fourth a square and fifth a promotion.
     * The function extracts the figure, starting square, target square, capture flag, promotion figure,
     * and move type from the input string and constructs a Move object with these values.
     *
     * @param INPUT The string representing the chess move.
     * @return The parsed Move object.
     */
    [[nodiscard]] Move parse_move(const std::string& INPUT) const;

    /**
     * @brief Returns the Zobrist hash of the current board state.
     *
     * The hash uniquely encodes piece placement, side to move, en-passant
     * file and castling rights, and is used for transposition table lookups
     * and repetition detection.
     *
     * @return 64-bit Zobrist hash of the position.
     */
    [[nodiscard]] uint64_t get_hash() const { return board_hash; }

private:

    /**
     * @brief Mailbox representation of the board (0..63).
     *
     * Each entry stores a Piece corresponding to a square on the board.
     */
    std::array<Piece, 64> board{Piece(EMPTY)};

    /**
     * @brief Precomputed Zobrist keys used to hash the board state.
     */
    const Zobrist ZOBRIST_TABLES;

    /**
     * @brief Cached Zobrist hash for the current position.
     */
    uint64_t board_hash{0};

    /**
     * @brief Recomputes the Zobrist hash from scratch based on the board state.
     *
     * Iterates over all squares, side to move, en-passant file and castling
     * rights to rebuild the 64-bit hash. Typically used after setting a
     * position from FEN.
     */
    void build_hash_for_board() {
        uint64_t hash = 0;
        // add all pieces with the square to the hash.
        for (int square = 0; square < 64; ++square) {
            if (const int PIECE = board[square].piece_type; PIECE != EMPTY) {
                hash ^= ZOBRIST_TABLES.zobrist_squares[PIECE][square];
            }
        }

        // Add stm to the hash.
        hash = hash ^ ZOBRIST_TABLES.zobrist_stm[player];

        // If ep square, get file and add hash.
        if (board_settings.ep_square != 100) {
            const int FILE = board_settings.ep_square % 8;
            hash ^= ZOBRIST_TABLES.zobrist_ep[FILE];
        }

        // Add each castling permission.
        if (board_settings.white_king_side) {
            hash ^= ZOBRIST_TABLES.zobrist_castling[0];
        }

        if (board_settings.white_queen_side) {
            hash ^= ZOBRIST_TABLES.zobrist_castling[1];
        }

        if (board_settings.black_king_side) {
            hash ^= ZOBRIST_TABLES.zobrist_castling[2];
        }
        if (board_settings.black_queen_side) {
            hash ^= ZOBRIST_TABLES.zobrist_castling[3];
        }

        board_hash = hash;
    }

    /**
     * @brief Updates the castling permissions after a move.
     *
     * This function updates the castling permissions on the board after a move.
     * It checks the type of the moving piece and the target square to determine which castling permissions should be
     * disabled.
     *
     * @param MOVE The move that was made.
     */
    void handle_castling_permissions(const Move& MOVE);

    /**
     * @brief Converts a square number to its corresponding X and Y coordinates on a chess board.
     *
     * Given a square number on a chess board, this function converts the square number to its
     * corresponding X and Y coordinates. The X-coordinate is represented by a letter from 'a' to 'h',
     * while the Y-coordinate is represented by a number from 1 to 8.
     *
     * @param SQUARE The square number on the chess board, ranging from 0 to 63.
     * @return A string representation of the X and Y coordinates in the format "Xn", where X is a letter
     *         and n is a number.
     */
    static std::string convert_to_x_and_y(const int SQUARE) {
        std::ostringstream out;
        out << static_cast<char>((SQUARE) % 8 + 'a') << (SQUARE) / 8 + 1;
        return out.str();
    }
};
