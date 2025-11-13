//
// Created by Marvin Becker on 05.03.24.
//
#include "./move_gen.h"

PseudoLegalMoves moveGenUtils::getAllPseudoLegalMoves(Board& board, bool player) {
    PseudoLegalMoves allPseudoMoves;
    // Go through the board with x and y coordinates.
    for (int y = 1; y < 9; y++) {
        for (int x = 1; x < 9; x++) {
            // Get the piece.
            Piece piece = board[calculateSquare(x, y)];
            if (piece.pieceType != EMPTY) {
                if ((piece.isWhite() && player) || (!piece.isWhite() && !player)) {
                    // Only get the players pieces.
                    switch (piece.pieceType) {
                        case BP:
                        case WP:
                            getAllPossiblePawnMoves({x, y}, board, allPseudoMoves, player);
                            break;
                        case BN:
                        case WN:
                            getAllPossibleKnightMoves({x, y}, board, allPseudoMoves, player);
                            break;
                        case BB:
                        case WB:
                            getAllPossibleBishopMoves({x, y}, board, allPseudoMoves, player);
                            break;
                        case BR:
                        case WR:
                            getAllPossibleRookMoves({x, y}, board, allPseudoMoves, player);
                            break;
                        case BQ:
                        case WQ:
                            getAllPossibleQueenMoves({x, y}, board, allPseudoMoves, player);
                            break;
                        case BK:
                        case WK:
                            getAllPossibleKingMoves({x, y}, board, allPseudoMoves, player);
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

template<int arraySize>
inline static void getAllLinearMoves(std::pair<int, int>& startSquare, Board& board, PseudoLegalMoves& allPseudoMoves,
                                     bool pieceColor, PieceType movingPiece,
                                     std::array<std::pair<int, int>, arraySize> directions) {
    // Calculate the square where the piece will be.
    int old_position = calculateSquare(startSquare.first, startSquare.second);

    // Build the move already.
    Move move{};
    move.square = old_position;
    move.movingPiece.pieceType = movingPiece;

    // Go through all directions.
    for (const auto& dir: directions) {
        int x = startSquare.first + dir.first;
        int y = startSquare.second + dir.second;

        // Every direction until the board ends.
        while (x > 0 && y > 0 && x < 9 && y < 9) {
            // Calculate the position.
            int position = calculateSquare(x, y);
            // Add to move.
            move.moveSquare = position;
            move.capturedPiece = board[position];

            // If move to an EMPTY field, just add it.
            if (board[position].pieceType == EMPTY) {
                allPseudoMoves.push_back(move);
            } else if (board[position].pieceType != EMPTY) {
                // If it is a piece.
                // Check if it is your own piece.
                if ((pieceColor && board[position].isWhite()) || (!pieceColor && !board[position].isWhite())) {
                    // Break the loop cause you cannot move further.
                    break;
                }
                // Add if enemy piece.
                allPseudoMoves.push_back(move);
                // Break the loop cause you cannot move further.
                break;
            }

            x += dir.first;
            y += dir.second;
        }
    }
}

void moveGenUtils::getAllPossibleRookMoves(std::pair<int, int> startSquare, Board& board,
                                           PseudoLegalMoves& allPseudoMoves, bool pieceColor) {
    // All directions a rook can move.
    std::array<std::pair<int, int>, 4> directions = {
        std::pair(-1, 0), std::pair(1, 0), std::pair(0, -1),
        std::pair(0, 1)
    };

    // Generate all the moves.
    getAllLinearMoves<4>(startSquare, board, allPseudoMoves, pieceColor, (pieceColor ? WR : BR), directions);
}

void moveGenUtils::getAllPossibleBishopMoves(std::pair<int, int> startSquare, Board& board,
                                             PseudoLegalMoves& allPseudoMoves, bool pieceColor) {
    // All directions a bishop can move.
    std::array<std::pair<int, int>, 4> directions = {
        std::pair(-1, -1), std::pair(-1, 1), std::pair(1, -1),
        std::pair(1, 1)
    };
    // Generate all the moves.
    getAllLinearMoves<4>(startSquare, board, allPseudoMoves, pieceColor, (pieceColor ? WB : BB), directions);
}

void moveGenUtils::getAllPossibleQueenMoves(std::pair<int, int> startSquare, Board& board,
                                            PseudoLegalMoves& allPseudoMoves, bool pieceColor) {
    // All directions a queen can move.
    std::array<std::pair<int, int>, 8> directions = {
        std::pair(-1, 0), std::pair(1, 0), std::pair(0, -1), std::pair(0, 1),
        std::pair(-1, -1), std::pair(-1, 1), std::pair(1, -1), std::pair(1, 1),
    };
    // Generate all the moves.
    getAllLinearMoves<8>(startSquare, board, allPseudoMoves, pieceColor, (pieceColor ? WQ : BQ), directions);
}

void moveGenUtils::getAllPossibleKingMoves(const std::pair<int, int>& startSquare, Board& board,
                                           PseudoLegalMoves& allPseudoMoves, bool pieceColor) {
    // Calculate current square.
    int old_position = calculateSquare(startSquare.first, startSquare.second);

    Move move{};
    move.square = old_position;
    move.movingPiece.pieceType = pieceColor ? WK : BK;

    // All directions a king can move.
    std::pair<int, int> directions[8] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};

    // Only run once. The king can only move one square.
    for (const auto& dir: directions) {
        int x = startSquare.first + dir.first;
        int y = startSquare.second + dir.second;
        int position = calculateSquare(x, y);

        // If it is in the board.
        if (x > 0 && y > 0 && x < 9 && y < 9) {
            Piece piece = board[position];
            if (piece.pieceType != EMPTY) {
                // Check if it is your own piece.
                if ((pieceColor && piece.isWhite()) || ((!pieceColor) && (!piece.isWhite()))) {
                    continue;
                }
            }
            move.moveSquare = position;
            move.capturedPiece = piece;
            allPseudoMoves.push_back(move);
        }
    }

    // Add casteling moves. King has to be save.
    if (!board.isKingInCheck(pieceColor)) {
        move.capturedPiece.pieceType = EMPTY;
        move.moveType = CASTLING;

        // KINGSIDE
        if ((pieceColor && board.boardSettings.whiteKingSide) || (!pieceColor && board.boardSettings.blackKingSide)) {
            int x = startSquare.first;
            int y = startSquare.second;
            bool canCastle = true;

            // Move two squares to the right.
            for (int i = 1; i < 3; i++) {
                int position = calculateSquare(x + i, y);

                // Check if the square is not empty or attacked.
                if (board[position].pieceType != EMPTY || board.isSquareAttacked({x + i, y}, pieceColor)) {
                    // Turn off casteling if so.
                    canCastle = false;
                    break;
                }
            }

            // After the full check if castling is still on add the move.
            if (canCastle) {
                move.moveSquare = calculateSquare(x + 2, y);
                allPseudoMoves.push_back(move);
            }
        }

        // QUEENSIDE
        if ((pieceColor && board.boardSettings.whiteQueenSide) || (!pieceColor && board.boardSettings.blackQueenSide)) {
            int x = startSquare.first;
            int y = startSquare.second;
            bool canCastle = true;

            // Move three squares to the left.
            for (int i = 1; i < 4; i++) {
                int position = calculateSquare(x - i, y);

                // If square is not empty castling is not allowed.
                if (board[position].pieceType != EMPTY) {
                    canCastle = false;
                    break;
                }

                // Only check the squares the king moves through if they are attacked.
                if (i < 3) {
                    if (board.isSquareAttacked({x - i, y}, pieceColor)) {
                        canCastle = false;
                        break;
                    }
                }
            }

            // After the full check if castling is still on add the move.
            if (canCastle) {
                move.moveSquare = calculateSquare(x - 2, y);
                allPseudoMoves.push_back(move);
            }
        }
    }
}

void moveGenUtils::getAllPossibleKnightMoves(const std::pair<int, int>& startSquare, Board& board,
                                             PseudoLegalMoves& allPseudoMoves, bool pieceColor) {
    // Calculate the original square.
    int old_position = calculateSquare(startSquare.first, startSquare.second);

    Move move{};
    move.square = old_position;
    move.movingPiece.pieceType = pieceColor ? WN : BN;

    // All the directions a knight can move to.
    std::pair<int, int> directions[8] = {{-2, -1}, {-1, -2}, {1, -2}, {2, -1}, {2, 1}, {1, 2}, {-1, 2}, {-2, 1}};

    // Knight can only move in 8 directions.
    for (const auto& dir: directions) {
        int x = startSquare.first + dir.first;
        int y = startSquare.second + dir.second;
        int position = calculateSquare(x, y);

        // If it is in the board.
        if (x > 0 && y > 0 && x < 9 && y < 9) {
            Piece piece = board[position];
            if (piece.pieceType != EMPTY) {
                // If there is a piece on that square.
                // Check if you capture your own piece.
                if ((pieceColor && piece.isWhite()) || ((!pieceColor) && (!piece.isWhite()))) {
                    continue;
                }
            }
            move.moveSquare = position;
            move.capturedPiece = board[position];
            allPseudoMoves.push_back(move);
        }
    }
}

void moveGenUtils::getAllPossiblePawnMoves(const std::pair<int, int>& startSquare, Board& board,
                                           PseudoLegalMoves& allPseudoMoves, bool pieceColor) {
    // Calculate original square.
    int old_position = calculateSquare(startSquare.first, startSquare.second);

    // All directions a pawn can move to including captures.
    std::pair<int, int> directions[4] = {{0, 1}, {-1, 1}, {1, 1}, {0, 2}};

    // Pawn only has 4 possible moves.
    for (const auto& dir: directions) {
        Move move{};
        move.square = old_position;
        move.movingPiece.pieceType = pieceColor ? WP : BP;

        // Calculate the square to move to.
        int x = startSquare.first + dir.first;
        int y = startSquare.second + dir.second;

        // If the pawn is black I need to negate it.
        if (!pieceColor) {
            x = startSquare.first - dir.first;
            y = startSquare.second - dir.second;
        }

        int position = calculateSquare(x, y);

        // If it is in the board.
        if (x > 0 && y > 0 && x < 9 && y < 9) {
            Piece piece = board[position];

            if (dir.first != 0 && dir.second != 0) {
                // If it is a diagonal jump.
                if (piece.pieceType != EMPTY) {
                    // If there is a piece.
                    // If there is your own piece.
                    if ((pieceColor && piece.isWhite()) || ((!pieceColor) && (!piece.isWhite()))) {
                        // Break the move.
                        continue;
                    }

                    // Check for promotion on rank 8 or 1 WITH capture.
                    if ((pieceColor && y == 8) || (!pieceColor && y == 1)) {
                        move.moveType = PROMOTION;
                        // Add all possible promotions.
                        for (int promotionIndex = 0; promotionIndex < 4; promotionIndex++) {
                            move.promotionPiece.pieceType = (pieceColor
                                                                 ? whitePawnPossiblePromotions[promotionIndex]
                                                                 : blackPawnPossiblePromotions[promotionIndex]);
                            move.moveSquare = position;
                            move.capturedPiece = board[position];
                            allPseudoMoves.push_back(move);
                        }
                        // Stop here before I add the move again.
                        continue;
                    }

                    // If there is no move to rank 8 or 1, just add the normal move.
                    move.moveSquare = position;
                    move.capturedPiece = board[position];
                    allPseudoMoves.push_back(move);
                } else {
                    if (board.boardSettings.epSquare != 100 &&
                        calculateSquare(x, y) == board.boardSettings.epSquare) {
                        // Check for EP
                        move.moveType = EN_PASSANT;
                        move.moveSquare = position;
                        move.capturedPiece = board[position];
                        allPseudoMoves.push_back(move);
                    }
                }
            } else if (dir.first == 0 && dir.second != 0 && piece.pieceType == EMPTY) {
                // If jump is just forward.
                if (dir.second == 2) {
                    if ((pieceColor && startSquare.second == 2) ||
                        (!pieceColor && startSquare.second == 7)) {
                        // Check if allowed to do the double jump.
                        if (board[calculateSquare(x, startSquare.second + (pieceColor ? 1 : -1))].pieceType ==
                            EMPTY) {
                            // Check if there is a piece on the destination.
                            move.moveSquare = position;
                            move.capturedPiece = board[position];
                            allPseudoMoves.push_back(move);
                        }
                    }
                    continue;
                }

                // Add promotions.
                if ((pieceColor && y == 8) || (!pieceColor && y == 1)) {
                    move.moveType = PROMOTION;
                    for (int promotionIndex = 0; promotionIndex < 4; promotionIndex++) {
                        move.promotionPiece.pieceType = (pieceColor
                                                             ? whitePawnPossiblePromotions[promotionIndex]
                                                             : blackPawnPossiblePromotions[promotionIndex]);
                        move.moveSquare = position;
                        move.capturedPiece = board[position];
                        allPseudoMoves.push_back(move);
                    }
                    continue;
                }

                move.moveSquare = position;
                move.capturedPiece = board[position];
                allPseudoMoves.push_back(move);
            }
        }
    }
}
