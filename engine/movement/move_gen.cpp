//
// Created by Marvin Becker on 05.03.24.
//
#include "./move_gen.h"
#include "../utils.h"

#include <iostream>

Move moveGenUtils::get_legal_fallback_move(Board& board)
{
    auto moves = get_all_pseudo_legal_moves(board, board.player_ == WHITE);

    for (Move& m : moves)
    {
        if (board.make_move(m))
        {
            board.pop_last_move();
            return m;
        }
    }

    // No legal moves (checkmate or stalemate).
    return Move{};
}

PseudoLegalMoves moveGenUtils::get_all_pseudo_legal_moves(Board& board, const bool player)
{
    PseudoLegalMoves allPseudoMoves;
    // Go through the board with x and y coordinates.
    for (int y = 1; y < 9; y++)
    {
        for (int x = 1; x < 9; x++)
        {
            // Get the piece.
            if (Piece piece = board[calculateSquare(x, y)]; piece.piece_type_ != EMPTY)
            {
                if ((piece.is_white() && player) || (!piece.is_white() && !player))
                {
                    // Only get the players pieces.
                    switch (piece.piece_type_)
                    {
                    case BP:
                    case WP:
                        get_all_possible_pawn_moves({x, y}, board, allPseudoMoves, player);
                        break;
                    case BN:
                    case WN:
                        get_all_possible_knight_moves({x, y}, board, allPseudoMoves, player);
                        break;
                    case BB:
                    case WB:
                        get_all_possible_bishop_moves({x, y}, board, allPseudoMoves, player);
                        break;
                    case BR:
                    case WR:
                        get_all_possible_rook_moves({x, y}, board, allPseudoMoves, player);
                        break;
                    case BQ:
                    case WQ:
                        get_all_possible_queen_moves({x, y}, board, allPseudoMoves, player);
                        break;
                    case BK:
                    case WK:
                        get_all_possible_king_moves({x, y}, board, allPseudoMoves, player);
                        break;
                    case EMPTY:
                        break;
                    }
                }
            }
        }
    }
    // Return all moves.
    return allPseudoMoves;
}

template <int arraySize>
static void getAllLinearMoves(std::pair<int, int>& start_square, Board& board,
                              PseudoLegalMoves& all_pseudo_moves, const bool piece_color,
                              const PieceType moving_piece,
                              std::array<std::pair<int, int>, arraySize> directions)
{
    // Calculate the square where the piece will be.
    const int OLD_POSITION = calculateSquare(start_square.first, start_square.second);

    // Build the move already.
    Move move{};
    move.square_ = OLD_POSITION;
    move.moving_piece_.piece_type_ = moving_piece;

    // Go through all directions.
    for (const auto& dir : directions)
    {
        int x = start_square.first + dir.first;
        int y = start_square.second + dir.second;

        // Every direction until the board ends.
        while (x > 0 && y > 0 && x < 9 && y < 9)
        {
            // Calculate the position.
            const int POSITION = calculateSquare(x, y);
            // Add to move.
            move.move_square_ = POSITION;
            move.captured_piece_ = board[POSITION];

            // If move to an EMPTY field, just add it.
            if (board[POSITION].piece_type_ == EMPTY)
                all_pseudo_moves.push_back(move);

            else if (board[POSITION].piece_type_ != EMPTY)
            {
                // If it is a piece.
                // Check if it is your own piece.
                if ((piece_color && board[POSITION].is_white()) ||
                    (!piece_color && !board[POSITION].is_white()))
                    // Break the loop cause you cannot move further.
                    break;

                // Add if enemy piece.
                all_pseudo_moves.push_back(move);
                // Break the loop cause you cannot move further.
                break;
            }

            x += dir.first;
            y += dir.second;
        }
    }
}

void moveGenUtils::get_all_possible_rook_moves(std::pair<int, int> start_square, Board& board,
                                               PseudoLegalMoves& all_pseudo_moves,
                                               const bool piece_color)
{
    // All directions a rook can move.
    constexpr std::array<std::pair<int, int>, 4> DIRECTIONS = {std::pair(-1, 0), std::pair(1, 0),
                                                               std::pair(0, -1), std::pair(0, 1)};

    // Generate all the moves.
    getAllLinearMoves<4>(start_square, board, all_pseudo_moves, piece_color,
                         (piece_color ? WR : BR), DIRECTIONS);
}

void moveGenUtils::get_all_possible_bishop_moves(std::pair<int, int> start_square, Board& board,
                                                 PseudoLegalMoves& all_pseudo_moves,
                                                 const bool piece_color)
{
    // All directions a bishop can move.
    constexpr std::array<std::pair<int, int>, 4> DIRECTIONS = {std::pair(-1, -1), std::pair(-1, 1),
                                                               std::pair(1, -1), std::pair(1, 1)};
    // Generate all the moves.
    getAllLinearMoves<4>(start_square, board, all_pseudo_moves, piece_color,
                         (piece_color ? WB : BB), DIRECTIONS);
}

void moveGenUtils::get_all_possible_queen_moves(std::pair<int, int> start_square, Board& board,
                                                PseudoLegalMoves& all_pseudo_moves,
                                                const bool piece_color)
{
    // All directions a queen can move.
    constexpr std::array<std::pair<int, int>, 8> DIRECTIONS = {
        std::pair(-1, 0),  std::pair(1, 0),  std::pair(0, -1), std::pair(0, 1),
        std::pair(-1, -1), std::pair(-1, 1), std::pair(1, -1), std::pair(1, 1),
    };
    // Generate all the moves.
    getAllLinearMoves<8>(start_square, board, all_pseudo_moves, piece_color,
                         (piece_color ? WQ : BQ), DIRECTIONS);
}

void moveGenUtils::get_all_possible_king_moves(const std::pair<int, int>& start_square,
                                               Board& board, PseudoLegalMoves& all_pseudo_moves,
                                               const bool piece_color)
{
    // Calculate current square.
    const int OLD_POSITION = calculateSquare(start_square.first, start_square.second);

    Move move{};
    move.square_ = OLD_POSITION;
    move.moving_piece_.piece_type_ = piece_color ? WK : BK;

    // All directions a king can move.
    std::pair<int, int> directions[8] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1},
                                         {0, 1},   {1, -1}, {1, 0},  {1, 1}};

    // Only run once. The king can only move one square.
    for (const auto& [FST, SND] : directions)
    {
        const int X = start_square.first + FST;
        const int Y = start_square.second + SND;
        const int POSITION = calculateSquare(X, Y);

        // If it is in the board.
        if (X > 0 && Y > 0 && X < 9 && Y < 9)
        {
            Piece piece = board[POSITION];
            if (piece.piece_type_ != EMPTY)
            {
                // Check if it is your own piece.
                if ((piece_color && piece.is_white()) || ((!piece_color) && (!piece.is_white())))
                    continue;
            }
            move.move_square_ = POSITION;
            move.captured_piece_ = piece;
            all_pseudo_moves.push_back(move);
        }
    }

    // Add casteling moves. King has to be save.
    if (!board.is_king_in_check(piece_color))
    {
        move.captured_piece_.piece_type_ = EMPTY;
        move.move_type_ = CASTLING;

        // KINGSIDE
        if ((piece_color && board.board_settings_.white_king_side_) ||
            (!piece_color && board.board_settings_.black_king_side_))
        {
            const int X = start_square.first;
            int y = start_square.second;
            bool canCastle = true;

            // Move two squares to the right.
            for (int i = 1; i < 3; i++)
            {

                // Check if the square is not empty or attacked.
                if (const int POSITION = calculateSquare(X + i, y);
                    board[POSITION].piece_type_ != EMPTY ||
                    board.is_square_attacked({X + i, y}, piece_color))
                {
                    // Turn off casteling if so.
                    canCastle = false;
                    break;
                }
            }

            // After the full check if castling is still on add the move.
            if (canCastle)
            {
                move.move_square_ = calculateSquare(X + 2, y);
                all_pseudo_moves.push_back(move);
            }
        }

        // QUEENSIDE
        if ((piece_color && board.board_settings_.white_queen_side_) ||
            (!piece_color && board.board_settings_.black_queen_side_))
        {
            const int X = start_square.first;
            int y = start_square.second;
            bool canCastle = true;

            // Move three squares to the left.
            for (int i = 1; i < 4; i++)
            {

                // If square is not empty castling is not allowed.
                if (const int POSITION = calculateSquare(X - i, y);
                    board[POSITION].piece_type_ != EMPTY)
                {
                    canCastle = false;
                    break;
                }

                // Only check the squares the king moves through if they are attacked.
                if (i < 3)
                {
                    if (board.is_square_attacked({X - i, y}, piece_color))
                    {
                        canCastle = false;
                        break;
                    }
                }
            }

            // After the full check if castling is still on add the move.
            if (canCastle)
            {
                move.move_square_ = calculateSquare(X - 2, y);
                all_pseudo_moves.push_back(move);
            }
        }
    }
}

void moveGenUtils::get_all_possible_knight_moves(const std::pair<int, int>& start_square,
                                                 Board& board, PseudoLegalMoves& all_pseudo_moves,
                                                 const bool piece_color)
{
    // Calculate the original square.
    const int OLD_POSITION = calculateSquare(start_square.first, start_square.second);

    Move move{};
    move.square_ = OLD_POSITION;
    move.moving_piece_.piece_type_ = piece_color ? WN : BN;

    // All the directions a knight can move to.
    std::pair<int, int> directions[8] = {{-2, -1}, {-1, -2}, {1, -2}, {2, -1},
                                         {2, 1},   {1, 2},   {-1, 2}, {-2, 1}};

    // Knight can only move in 8 directions.
    for (const auto& [FST, SND] : directions)
    {
        const int X = start_square.first + FST;
        const int Y = start_square.second + SND;
        const int POSITION = calculateSquare(X, Y);

        // If it is in the board.
        if (X > 0 && Y > 0 && X < 9 && Y < 9)
        {
            if (Piece piece = board[POSITION]; piece.piece_type_ != EMPTY)
            {
                // If there is a piece on that square.
                // Check if you capture your own piece.
                if ((piece_color && piece.is_white()) || ((!piece_color) && (!piece.is_white())))
                    continue;
            }
            move.move_square_ = POSITION;
            move.captured_piece_ = board[POSITION];
            all_pseudo_moves.push_back(move);
        }
    }
}

void moveGenUtils::get_all_possible_pawn_moves(const std::pair<int, int>& start_square,
                                               Board& board, PseudoLegalMoves& all_pseudo_moves,
                                               const bool piece_color)
{
    // Calculate original square.
    const int OLD_POSITION = calculateSquare(start_square.first, start_square.second);

    // All directions a pawn can move to including captures.
    std::pair<int, int> directions[4] = {{0, 1}, {-1, 1}, {1, 1}, {0, 2}};

    // Pawn only has 4 possible moves.
    for (const auto& [FST, SND] : directions)
    {
        Move move{};
        move.square_ = OLD_POSITION;
        move.moving_piece_.piece_type_ = piece_color ? WP : BP;

        // Calculate the square to move to.
        int x = start_square.first + FST;
        int y = start_square.second + SND;

        // If the pawn is black I need to negate it.
        if (!piece_color)
        {
            x = start_square.first - FST;
            y = start_square.second - SND;
        }

        const int POSITION = calculateSquare(x, y);

        // If it is in the board.
        if (x > 0 && y > 0 && x < 9 && y < 9)
        {
            Piece piece = board[POSITION];

            if (FST != 0 && SND != 0)
            {
                // If it is a diagonal jump.
                if (piece.piece_type_ != EMPTY)
                {
                    // If there is a piece.
                    // If there is your own piece.
                    if ((piece_color && piece.is_white()) ||
                        ((!piece_color) && (!piece.is_white())))
                        // Break the move.
                        continue;

                    // Check for promotion on rank 8 or 1 WITH capture.
                    if ((piece_color && y == 8) || (!piece_color && y == 1))
                    {
                        move.move_type_ = PROMOTION;
                        // Add all possible promotions.
                        for (int promotionIndex = 0; promotionIndex < 4; promotionIndex++)
                        {
                            move.promotion_piece_.piece_type_ =
                                (piece_color ? WHITE_PAWN_POSSIBLE_PROMOTIONS[promotionIndex]
                                             : BLACK_PAWN_POSSIBLE_PROMOTIONS[promotionIndex]);
                            move.move_square_ = POSITION;
                            move.captured_piece_ = board[POSITION];
                            all_pseudo_moves.push_back(move);
                        }
                        // Stop here before I add the move again.
                        continue;
                    }

                    // If there is no move to rank 8 or 1, just add the normal move.
                    move.move_square_ = POSITION;
                    move.captured_piece_ = board[POSITION];
                    all_pseudo_moves.push_back(move);
                }
                else
                {
                    if (board.board_settings_.ep_square_ != 100 &&
                        calculateSquare(x, y) == board.board_settings_.ep_square_)
                    {
                        // Check for EP
                        move.move_type_ = EN_PASSANT;
                        move.move_square_ = POSITION;
                        move.captured_piece_ = board[POSITION];
                        all_pseudo_moves.push_back(move);
                    }
                }
            }
            else if (FST == 0 && SND != 0 && piece.piece_type_ == EMPTY)
            {
                // If jump is just forward.
                if (SND == 2)
                {
                    if ((piece_color && start_square.second == 2) ||
                        (!piece_color && start_square.second == 7))
                    {
                        // Check if allowed to do the double jump.
                        if (board[calculateSquare(x, start_square.second + (piece_color ? 1 : -1))]
                                .piece_type_ == EMPTY)
                        {
                            // Check if there is a piece on the destination.
                            move.move_square_ = POSITION;
                            move.captured_piece_ = board[POSITION];
                            all_pseudo_moves.push_back(move);
                        }
                    }
                    continue;
                }

                // Add promotions.
                if ((piece_color && y == 8) || (!piece_color && y == 1))
                {
                    move.move_type_ = PROMOTION;
                    for (int promotionIndex = 0; promotionIndex < 4; promotionIndex++)
                    {
                        move.promotion_piece_.piece_type_ =
                            (piece_color ? WHITE_PAWN_POSSIBLE_PROMOTIONS[promotionIndex]
                                         : BLACK_PAWN_POSSIBLE_PROMOTIONS[promotionIndex]);
                        move.move_square_ = POSITION;
                        move.captured_piece_ = board[POSITION];
                        all_pseudo_moves.push_back(move);
                    }
                    continue;
                }

                move.move_square_ = POSITION;
                move.captured_piece_ = board[POSITION];
                all_pseudo_moves.push_back(move);
            }
        }
    }
}
