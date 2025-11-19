//
// Created by Marvin Becker on 15.12.23.
//

#pragma once

#include <cassert>
#include <map>
#include <random>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "./misc/board_settings.h"
#include "./misc/move.h"
#include "./misc/piece.h"
#include "./misc/player.h"
#include "./misc/zobrist.h"

/**
 * @brief Calculates the square in the board on the x and y coordinates.
 *
 * The position is calculated using the formula: ((y - 1) * 8 + x) - 1.
 * The grid has 8 columns and the indexing starts from 0, so the result is subtracted by 1.
 *
 * @param x The x-coordinate.
 * @param y The y-coordinate.
 * @return The position in the grid.
 */
inline int calculateSquare(int x, int y) { return ((y - 1) * 8 + x) - 1; }

/**
 * @class Board
 * @brief Represents a chessboard.
 *
 * The Board class represents a chessboard and provides functions for managing the state of the board and making moves.
 */
class Board {
public:
    // Represents the current side to move.
    player_type player{WHITE};
    // Represents all moves so far.
    std::vector<Move> moves;
    // Represents all settings so far.
    std::vector<board_setting> history;
    // Represents the current settings.
    board_setting boardSettings;

    // To access the board at the given index directly.
    Piece& operator[](int index) {
        assert(index < 64 && index >= 0);
        return board[index];
    }

    /**
     * @brief Initializes a new Board object.
     *
     * This constructor initializes a new Board object. It sets the current player to WHITE and initializes
     * each piece on the board to an empty piece. It then reads the initial board state from a FEN string.
     */
    Board() : player(WHITE), board{Piece(EMPTY)} {
        readFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    };

    /**
     * @brief Checks if the king of the given piece color is in check.
     *
     * This function iterates through the board to find the king of the given piece color.
     * Once the king is found, it calls the isSquareAttacked function to check if the king is being attacked.
     *
     * @param pieceColor The color of the king to check (true for white, false for black).
     * @return True if the king is in check, false otherwise.
     */
    bool isKingInCheck(bool pieceColor);

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
     * @param square The square to check.
     * @param pieceColor The color of the piece to check (true for white, false for black).
     * @return True if the square is attacked, false otherwise.
     */
    bool isSquareAttacked(const std::pair<int, int>& square, bool pieceColor);

    /**
     * @brief Attempts to move a chess piece on the board.
     *
     * This function attempts to move a chess piece on the board. It checks various conditions such as legality of the
     * move and checks for pieces belonging to the opponent. If the move is successful, it updates the board state
     * accordingly.
     *
     * @param move The move to be made.
     * @return True if the move is successful, false otherwise. If false, the move will not be applied.
     */
    bool tryToMovePiece(const Move& move);

    /**
     * @brief Moves a chess piece on the board.
     *
     * This function moves a chess piece on the board. It only checks if the move is legal (you don't set yourself in
     * check) and updates the board accordingly. This function does not check if the move is correct. Use @see
     * tryToMovePiece for correct checking. The function returns true if the move is successful, and false otherwise. If
     * false the move will not be applied!
     *
     * @param move The move to make.
     * @return True if the move is successful, false otherwise. If false the move will not be applied!.
     */
    bool makeMove(const Move& move);

    /**
     * @brief Removes the last move from the list of moves and updates the board state accordingly.
     *
     * This function removes the last move from the list of moves and updates the board state by reversing the changes
     * made by the move. It returns true if the move was successfully undone, and false otherwise.
     *
     * @return True if the last move was successfully undone, false otherwise.
     */
    bool popLastMove();

    /**
     * @brief Prints the current state of the chessboard.
     *
     * This function prints the current state of the chessboard.
     * It shows the current turn and the positions of all the pieces on the board.
     */
    void printCurrentBoard() const;

    /**
     * @brief Reads a FEN string and sets up the board accordingly.
     *
     * This function reads a FEN (Forsyth-Edwards Notation) string and sets up the chessboard based on the provided FEN
     * string. Then FEN has to be correct. Otherwise the programm will crash!
     *
     * @param input The FEN string to parse.
     */
    void readFen(const std::string& input);

    /**
     * @brief Returns the current FEN representation of the chessboard.
     *
     * This function returns the current FEN (Forsyth-Edwards Notation) representation of the chessboard.
     * The FEN string represents the board state, the current player to move, castling rights and the en passant target
     * square.
     *
     * @return The current FEN representation of the chessboard.
     */
    [[nodiscard]] std::string getFen() const;

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
    bool isCheckMate(bool isWhite);

    /**
     * @brief Parses a chess move from a string input.
     *
     * This function takes a string input representing a chess move and parses it into a Move object.
     * The string input should follow this notation: Pa2(x)a3(=Q).
     * First the piece, second the square, third capture is optional, fourth a square and fifth a promotion.
     * The function extracts the figure, starting square, target square, capture flag, promotion figure,
     * and move type from the input string and constructs a Move object with these values.
     *
     * @param input The string representing the chess move.
     * @return The parsed Move object.
     */
    [[nodiscard]] Move parseMove(const std::string& input) const;

    [[nodiscard]] uint64_t getHash() const { return boardHash; }

private:
    // Represents the board.
    std::array<Piece, 64> board{Piece(EMPTY)};
    const Zobrist zobristTables;
    uint64_t boardHash{0};

    void buildHashForBoard() {
        uint64_t hash = 0;
        // add all pieces with the square to the hash.
        for (int square = 0; square < 64; ++square) {
            int piece = board[square].pieceType;
            if (piece != EMPTY) {
                hash ^= zobristTables.zobrist_squares[piece][square];
            }
        }

        // Add stm to the hash.
        hash = hash ^ zobristTables.zobrist_stm[player];

        // If ep square, get file and add hash.
        if (boardSettings.epSquare != 100) {
            int file = boardSettings.epSquare % 8;
            hash ^= zobristTables.zobrist_ep[file];
        }

        // Add each castling permission.
        if (boardSettings.whiteKingSide) {
            hash ^= zobristTables.zobrist_castling[0];
        }

        if (boardSettings.whiteQueenSide) {
            hash ^= zobristTables.zobrist_castling[1];
        }

        if (boardSettings.blackKingSide) {
            hash ^= zobristTables.zobrist_castling[2];
        }
        if (boardSettings.blackQueenSide) {
            hash ^= zobristTables.zobrist_castling[3];
        }

        boardHash = hash;
    }

    /**
     * @brief Updates the castling permissions after a move.
     *
     * This function updates the castling permissions on the board after a move.
     * It checks the type of the moving piece and the target square to determine which castling permissions should be
     * disabled.
     *
     * @param move The move that was made.
     */
    void handleCastlingPermissions(const Move& move);

    /**
     * @brief Converts a square number to its corresponding X and Y coordinates on a chess board.
     *
     * Given a square number on a chess board, this function converts the square number to its
     * corresponding X and Y coordinates. The X-coordinate is represented by a letter from 'a' to 'h',
     * while the Y-coordinate is represented by a number from 1 to 8.
     *
     * @param square The square number on the chess board, ranging from 0 to 63.
     * @return A string representation of the X and Y coordinates in the format "Xn", where X is a letter
     *         and n is a number.
     */
    static std::string convertToXandY(const int square) {
        std::ostringstream out;
        out << static_cast<char>((square) % 8 + 'a') << (square) / 8 + 1;
        return out.str();
    }
};
