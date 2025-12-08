//
// Created by Marvin Becker on 05.03.24.
//
#include "./move_gen.h"

#include "../utils.h"

PseudoLegalMoves moveGenUtils::get_all_pseudo_legal_moves(Board& board, const bool PLAYER)
{
    PseudoLegalMoves allPseudoMoves;
    // Go through the board with x and y coordinates.
    for (int y = 1; y < 9; y++)
    {
        for (int x = 1; x < 9; x++)
        {
            // Get the piece.
            if (Piece piece = board[calculateSquare(x, y)]; piece.piece_type != EMPTY)
            {
                if ((piece.is_white() && PLAYER) || (!piece.is_white() && !PLAYER))
                {
                    // Only get the players pieces.
                    switch (piece.piece_type)
                    {
                    case BP:
                    case WP:
                        get_all_possible_pawn_moves({x, y}, board, allPseudoMoves, PLAYER);
                        break;
                    case BN:
                    case WN:
                        get_all_possible_knight_moves({x, y}, board, allPseudoMoves, PLAYER);
                        break;
                    case BB:
                    case WB:
                        get_all_possible_bishop_moves({x, y}, board, allPseudoMoves, PLAYER);
                        break;
                    case BR:
                    case WR:
                        get_all_possible_rook_moves({x, y}, board, allPseudoMoves, PLAYER);
                        break;
                    case BQ:
                    case WQ:
                        get_all_possible_queen_moves({x, y}, board, allPseudoMoves, PLAYER);
                        break;
                    case BK:
                    case WK:
                        get_all_possible_king_moves({x, y}, board, allPseudoMoves, PLAYER);
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
                              PseudoLegalMoves& all_pseudo_moves, const bool PIECE_COLOR,
                              const PieceType MOVING_PIECE,
                              std::array<std::pair<int, int>, arraySize> directions)
{
    // Calculate the square where the piece will be.
    const int OLD_POSITION = calculateSquare(start_square.first, start_square.second);

    // Build the move already.
    Move move{};
    move.square = OLD_POSITION;
    move.moving_piece.piece_type = MOVING_PIECE;

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
            move.move_square = POSITION;
            move.captured_piece = board[POSITION];

            // If move to an EMPTY field, just add it.
            if (board[POSITION].piece_type == EMPTY)
            {
                all_pseudo_moves.push_back(move);
            }
            else if (board[POSITION].piece_type != EMPTY)
            {
                // If it is a piece.
                // Check if it is your own piece.
                if ((PIECE_COLOR && board[POSITION].is_white()) ||
                    (!PIECE_COLOR && !board[POSITION].is_white()))
                {
                    // Break the loop cause you cannot move further.
                    break;
                }
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
                                               const bool PIECE_COLOR)
{
    // All directions a rook can move.
    constexpr std::array<std::pair<int, int>, 4> DIRECTIONS = {std::pair(-1, 0), std::pair(1, 0),
                                                               std::pair(0, -1), std::pair(0, 1)};

    // Generate all the moves.
    getAllLinearMoves<4>(start_square, board, all_pseudo_moves, PIECE_COLOR,
                         (PIECE_COLOR ? WR : BR), DIRECTIONS);
}

void moveGenUtils::get_all_possible_bishop_moves(std::pair<int, int> start_square, Board& board,
                                                 PseudoLegalMoves& all_pseudo_moves,
                                                 const bool PIECE_COLOR)
{
    // All directions a bishop can move.
    constexpr std::array<std::pair<int, int>, 4> DIRECTIONS = {std::pair(-1, -1), std::pair(-1, 1),
                                                               std::pair(1, -1), std::pair(1, 1)};
    // Generate all the moves.
    getAllLinearMoves<4>(start_square, board, all_pseudo_moves, PIECE_COLOR,
                         (PIECE_COLOR ? WB : BB), DIRECTIONS);
}

void moveGenUtils::get_all_possible_queen_moves(std::pair<int, int> start_square, Board& board,
                                                PseudoLegalMoves& all_pseudo_moves,
                                                const bool PIECE_COLOR)
{
    // All directions a queen can move.
    constexpr std::array<std::pair<int, int>, 8> DIRECTIONS = {
        std::pair(-1, 0),  std::pair(1, 0),  std::pair(0, -1), std::pair(0, 1),
        std::pair(-1, -1), std::pair(-1, 1), std::pair(1, -1), std::pair(1, 1),
    };
    // Generate all the moves.
    getAllLinearMoves<8>(start_square, board, all_pseudo_moves, PIECE_COLOR,
                         (PIECE_COLOR ? WQ : BQ), DIRECTIONS);
}

void moveGenUtils::get_all_possible_king_moves(const std::pair<int, int>& START_SQUARE,
                                               Board& board, PseudoLegalMoves& all_pseudo_moves,
                                               const bool PIECE_COLOR)
{
    // Calculate current square.
    const int OLD_POSITION = calculateSquare(START_SQUARE.first, START_SQUARE.second);

    Move move{};
    move.square = OLD_POSITION;
    move.moving_piece.piece_type = PIECE_COLOR ? WK : BK;

    // All directions a king can move.
    std::pair<int, int> directions[8] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1},
                                         {0, 1},   {1, -1}, {1, 0},  {1, 1}};

    // Only run once. The king can only move one square.
    for (const auto& [FST, SND] : directions)
    {
        int x = START_SQUARE.first + FST;
        int y = START_SQUARE.second + SND;
        int position = calculateSquare(x, y);

        // If it is in the board.
        if (x > 0 && y > 0 && x < 9 && y < 9)
        {
            Piece piece = board[position];
            if (piece.piece_type != EMPTY)
            {
                // Check if it is your own piece.
                if ((PIECE_COLOR && piece.is_white()) || ((!PIECE_COLOR) && (!piece.is_white())))
                {
                    continue;
                }
            }
            move.move_square = position;
            move.captured_piece = piece;
            all_pseudo_moves.push_back(move);
        }
    }

    // Add casteling moves. King has to be save.
    if (!board.is_king_in_check(PIECE_COLOR))
    {
        move.captured_piece.piece_type = EMPTY;
        move.move_type = CASTLING;

        // KINGSIDE
        if ((PIECE_COLOR && board.board_settings.white_king_side) ||
            (!PIECE_COLOR && board.board_settings.black_king_side))
        {
            const int X = START_SQUARE.first;
            int y = START_SQUARE.second;
            bool canCastle = true;

            // Move two squares to the right.
            for (int i = 1; i < 3; i++)
            {
                int position = calculateSquare(X + i, y);

                // Check if the square is not empty or attacked.
                if (board[position].piece_type != EMPTY ||
                    board.is_square_attacked({X + i, y}, PIECE_COLOR))
                {
                    // Turn off casteling if so.
                    canCastle = false;
                    break;
                }
            }

            // After the full check if castling is still on add the move.
            if (canCastle)
            {
                move.move_square = calculateSquare(X + 2, y);
                all_pseudo_moves.push_back(move);
            }
        }

        // QUEENSIDE
        if ((PIECE_COLOR && board.board_settings.white_queen_side) ||
            (!PIECE_COLOR && board.board_settings.black_queen_side))
        {
            const int X = START_SQUARE.first;
            int y = START_SQUARE.second;
            bool canCastle = true;

            // Move three squares to the left.
            for (int i = 1; i < 4; i++)
            {
                int position = calculateSquare(X - i, y);

                // If square is not empty castling is not allowed.
                if (board[position].piece_type != EMPTY)
                {
                    canCastle = false;
                    break;
                }

                // Only check the squares the king moves through if they are attacked.
                if (i < 3)
                {
                    if (board.is_square_attacked({X - i, y}, PIECE_COLOR))
                    {
                        canCastle = false;
                        break;
                    }
                }
            }

            // After the full check if castling is still on add the move.
            if (canCastle)
            {
                move.move_square = calculateSquare(X - 2, y);
                all_pseudo_moves.push_back(move);
            }
        }
    }
}

void moveGenUtils::get_all_possible_knight_moves(const std::pair<int, int>& START_SQUARE,
                                                 Board& board, PseudoLegalMoves& all_pseudo_moves,
                                                 const bool PIECE_COLOR)
{
    // Calculate the original square.
    const int old_position = calculateSquare(START_SQUARE.first, START_SQUARE.second);

    Move move{};
    move.square = old_position;
    move.moving_piece.piece_type = PIECE_COLOR ? WN : BN;

    // All the directions a knight can move to.
    std::pair<int, int> directions[8] = {{-2, -1}, {-1, -2}, {1, -2}, {2, -1},
                                         {2, 1},   {1, 2},   {-1, 2}, {-2, 1}};

    // Knight can only move in 8 directions.
    for (const auto& [FST, SND] : directions)
    {
        const int X = START_SQUARE.first + FST;
        const int Y = START_SQUARE.second + SND;
        const int POSITION = calculateSquare(X, Y);

        // If it is in the board.
        if (X > 0 && Y > 0 && X < 9 && Y < 9)
        {
            if (Piece piece = board[POSITION]; piece.piece_type != EMPTY)
            {
                // If there is a piece on that square.
                // Check if you capture your own piece.
                if ((PIECE_COLOR && piece.is_white()) || ((!PIECE_COLOR) && (!piece.is_white())))
                {
                    continue;
                }
            }
            move.move_square = POSITION;
            move.captured_piece = board[POSITION];
            all_pseudo_moves.push_back(move);
        }
    }
}

void moveGenUtils::get_all_possible_pawn_moves(const std::pair<int, int>& START_SQUARE,
                                               Board& board, PseudoLegalMoves& all_pseudo_moves,
                                               const bool PIECE_COLOR)
{
    // Calculate original square.
    const int OLD_POSITION = calculateSquare(START_SQUARE.first, START_SQUARE.second);

    // All directions a pawn can move to including captures.
    std::pair<int, int> directions[4] = {{0, 1}, {-1, 1}, {1, 1}, {0, 2}};

    // Pawn only has 4 possible moves.
    for (const auto& [FST, SND] : directions)
    {
        Move move{};
        move.square = OLD_POSITION;
        move.moving_piece.piece_type = PIECE_COLOR ? WP : BP;

        // Calculate the square to move to.
        int x = START_SQUARE.first + FST;
        int y = START_SQUARE.second + SND;

        // If the pawn is black I need to negate it.
        if (!PIECE_COLOR)
        {
            x = START_SQUARE.first - FST;
            y = START_SQUARE.second - SND;
        }

        const int POSITION = calculateSquare(x, y);

        // If it is in the board.
        if (x > 0 && y > 0 && x < 9 && y < 9)
        {
            Piece piece = board[POSITION];

            if (FST != 0 && SND != 0)
            {
                // If it is a diagonal jump.
                if (piece.piece_type != EMPTY)
                {
                    // If there is a piece.
                    // If there is your own piece.
                    if ((PIECE_COLOR && piece.is_white()) ||
                        ((!PIECE_COLOR) && (!piece.is_white())))
                    {
                        // Break the move.
                        continue;
                    }

                    // Check for promotion on rank 8 or 1 WITH capture.
                    if ((PIECE_COLOR && y == 8) || (!PIECE_COLOR && y == 1))
                    {
                        move.move_type = PROMOTION;
                        // Add all possible promotions.
                        for (int promotionIndex = 0; promotionIndex < 4; promotionIndex++)
                        {
                            move.promotion_piece.piece_type =
                                (PIECE_COLOR ? white_pawn_possible_promotions[promotionIndex]
                                             : black_pawn_possible_promotions[promotionIndex]);
                            move.move_square = POSITION;
                            move.captured_piece = board[POSITION];
                            all_pseudo_moves.push_back(move);
                        }
                        // Stop here before I add the move again.
                        continue;
                    }

                    // If there is no move to rank 8 or 1, just add the normal move.
                    move.move_square = POSITION;
                    move.captured_piece = board[POSITION];
                    all_pseudo_moves.push_back(move);
                }
                else
                {
                    if (board.board_settings.ep_square != 100 &&
                        calculateSquare(x, y) == board.board_settings.ep_square)
                    {
                        // Check for EP
                        move.move_type = EN_PASSANT;
                        move.move_square = POSITION;
                        move.captured_piece = board[POSITION];
                        all_pseudo_moves.push_back(move);
                    }
                }
            }
            else if (FST == 0 && SND != 0 && piece.piece_type == EMPTY)
            {
                // If jump is just forward.
                if (SND == 2)
                {
                    if ((PIECE_COLOR && START_SQUARE.second == 2) ||
                        (!PIECE_COLOR && START_SQUARE.second == 7))
                    {
                        // Check if allowed to do the double jump.
                        if (board[calculateSquare(x, START_SQUARE.second + (PIECE_COLOR ? 1 : -1))]
                                .piece_type == EMPTY)
                        {
                            // Check if there is a piece on the destination.
                            move.move_square = POSITION;
                            move.captured_piece = board[POSITION];
                            all_pseudo_moves.push_back(move);
                        }
                    }
                    continue;
                }

                // Add promotions.
                if ((PIECE_COLOR && y == 8) || (!PIECE_COLOR && y == 1))
                {
                    move.move_type = PROMOTION;
                    for (int promotionIndex = 0; promotionIndex < 4; promotionIndex++)
                    {
                        move.promotion_piece.piece_type =
                            (PIECE_COLOR ? white_pawn_possible_promotions[promotionIndex]
                                         : black_pawn_possible_promotions[promotionIndex]);
                        move.move_square = POSITION;
                        move.captured_piece = board[POSITION];
                        all_pseudo_moves.push_back(move);
                    }
                    continue;
                }

                move.move_square = POSITION;
                move.captured_piece = board[POSITION];
                all_pseudo_moves.push_back(move);
            }
        }
    }
}
