//
// Created by Marvin Becker on 15.12.23.
//

#include <iostream>

#include "./board.h"
#include "./chess_bot.h"
#include "./exceptions/board_exception.h"
#include "./exceptions/fen_exception.h"

bool Board::isKingInCheck(bool pieceColor) {
    // Go through board.
    for (int y = 8; y >= 1; y--) {
        for (int x = 1; x <= 8; x++) {
            if (board[calculateSquare(x, y)].pieceType == (pieceColor ? WK : BK)) {
                // If piece is the correct King.
                // Check if the king square is attacked.
                return isSquareAttacked({x, y}, pieceColor);
            }
        }
    }

    // If no king is found throw exception.
    throw BoardInterruptException("No king found!");
}

bool Board::isSquareAttacked(const std::pair<int, int>& square, bool pieceColor) {
    PseudoLegalMoves allKnightMoves;
    PseudoLegalMoves allPawnMoves;
    PseudoLegalMoves allBishopMoves;
    PseudoLegalMoves allRookMoves;

    // getALlPossible###Moves does only return captures of the opponent pieces. So no need to check again if you capture
    // your own piece

    moveGenUtils::getAllPossibleKnightMoves(square, *this, allKnightMoves, pieceColor);
    for (Move& move: allKnightMoves) {
        if (move.capturedPiece.pieceType == WN || move.capturedPiece.pieceType == BN) {
            return true;
        }
    }

    moveGenUtils::getAllPossiblePawnMoves(square, *this, allPawnMoves, pieceColor);
    for (Move& move: allPawnMoves) {
        if (move.capturedPiece.pieceType == WP || move.capturedPiece.pieceType == BP) {
            return true;
        }
    }

    // You also have to check for Queens because they can move like the bishop too!
    moveGenUtils::getAllPossibleBishopMoves(square, *this, allBishopMoves, pieceColor);
    for (Move& move: allBishopMoves) {
        if (move.capturedPiece.pieceType == WB || move.capturedPiece.pieceType == BB ||
            move.capturedPiece.pieceType == WQ || move.capturedPiece.pieceType == BQ) {
            return true;
        }
    }

    // You also have to check for Queens because they can move like the rook too!
    moveGenUtils::getAllPossibleRookMoves(square, *this, allRookMoves, pieceColor);
    for (Move& move: allRookMoves) {
        if (move.capturedPiece.pieceType == WR || move.capturedPiece.pieceType == BR ||
            move.capturedPiece.pieceType == WQ || move.capturedPiece.pieceType == BQ) {
            return true;
        }
    }

    // Specifically check if there is a king near you.
    std::pair<int, int> directions[8] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};
    for (const auto& dir: directions) {
        int x = square.first + dir.first;
        int y = square.second + dir.second;
        if (x > 0 && y > 0 && x < 9 && y < 9) {
            if (board[calculateSquare(x, y)].pieceType == (pieceColor ? BK : WK)) {
                return true;
            }
        }
    }

    return false;
}

bool Board::popLastMove() {
    // You shouldn't be able to pop if there is nothing.
    if (moves.empty() || history.empty()) {
        return false;
    }

    // Save last move.
    Move last_move = moves.back();
    // Pop it.
    moves.pop_back();
    // Pop the history.
    history.pop_back();
    // Set the piece which got caught on the moved square.
    board[last_move.moveSquare] = last_move.capturedPiece;
    // Set the moved piece on its original position.
    board[last_move.square] = last_move.movingPiece;

    if (last_move.moveType == EN_PASSANT) {
        // If the move was an EP.
        // Get the square behind the ep square by adding or subtracting 8 (One row).
        int enPassantSquare = last_move.moveSquare + (last_move.movingPiece.isWhite() ? -8 : +8);
        // Set the piece which got caught there.
        board[enPassantSquare].pieceType = (last_move.movingPiece.isWhite() ? BP : WP);
    }

    if (last_move.moveType == CASTLING) {
        // If the move was castling.
        if (last_move.moveSquare == 6) {
            // Check if the King moved to square g1.
            // Reset the rook to h1.
            board[7].pieceType = WR;
            board[5].pieceType = EMPTY;
        }
        if (last_move.moveSquare == 2) {
            // Check if the King moved to square c1.
            // Reset the rook to a1.
            board[0].pieceType = WR;
            board[3].pieceType = EMPTY;
        }
        if (last_move.moveSquare == 62) {
            // Check if the King moved to square g8.
            // Reset the rook to h8.
            board[63].pieceType = BR;
            board[61].pieceType = EMPTY;
        }
        if (last_move.moveSquare == 58) {
            // Check if the King moved to square c8.
            // Reset the rook to h8.
            board[56].pieceType = BR;
            board[59].pieceType = EMPTY;
        }
    }

    // Set player back.
    player = player == WHITE ? BLACK : WHITE;

    // Settings reset.
    boardSettings = history.back();

    buildHashForBoard();
    return true;
}

bool Board::makeMove(const Move& move) {
    // Set the square to move to the piece where it is currently.
    board[move.moveSquare] = board[move.square];

    // Set on the old square an EMPTY piece.
    board[move.square].pieceType = EMPTY;

    // Reset the eqSquare.
    boardSettings.epSquare = 100;
    // Increment lastMoveSincePawnOrCapture.
    boardSettings.lastMovesSincePawnOrCapture++;

    // If move is a promotion set on the future square the promotion piece.
    if (move.moveType == PROMOTION) {
        board[move.moveSquare] = move.promotionPiece;
    }

    // If move is EP then replace the square in front of the move with empty.
    if (move.moveType == EN_PASSANT) {
        int enPassantSquare = move.moveSquare + (move.movingPiece.pieceType == WP ? -8 : +8);
        board[enPassantSquare].pieceType = EMPTY;
    }

    // If it is a castling move just set the rook at the correct spot.
    if (move.moveType == CASTLING) {
        if (move.moveSquare == 6) {
            board[7].pieceType = EMPTY;
            board[5].pieceType = WR;
        }
        if (move.moveSquare == 2) {
            board[0].pieceType = EMPTY;
            board[3].pieceType = WR;
        }
        if (move.moveSquare == 62) {
            board[63].pieceType = EMPTY;
            board[61].pieceType = BR;
        }
        if (move.moveSquare == 58) {
            board[56].pieceType = EMPTY;
            board[59].pieceType = BR;
        }
    }

    if (move.movingPiece.pieceType == WP || move.movingPiece.pieceType == BP) {
        if (std::abs(move.square - move.moveSquare) == 16) {
            // Set EP square if a pawn moves exact 2 rows.
            boardSettings.epSquare = move.moveSquare + (move.movingPiece.isWhite() ? -8 : +8);
        }
    }

    // Set the permissions for castling!
    handleCastlingPermissions(move);

    if (player == BLACK) {
        boardSettings.turns++;
    }

    if (((move.movingPiece.pieceType == WP) || (move.movingPiece.pieceType == BP)) ||
        move.capturedPiece.pieceType != EMPTY) {
        boardSettings.lastMovesSincePawnOrCapture = 0;
    }

    // Save move
    moves.push_back(move);
    // Save settings
    history.push_back(boardSettings);
    // Reset the player.
    player = player == WHITE ? BLACK : WHITE;

    // Check if your king is in check after the move and pop if yes.
    if (isKingInCheck(player != WHITE)) {
        popLastMove();
        return false;
    }

    buildHashForBoard();

    // return true if everything is fine.
    return true;
}

void Board::handleCastlingPermissions(const Move& move) {
    // If king is moved. disable everything.
    if (move.movingPiece.pieceType == WK) {
        boardSettings.whiteQueenSide = false;
        boardSettings.whiteKingSide = false;
    } else if (move.movingPiece.pieceType == BK) {
        boardSettings.blackQueenSide = false;
        boardSettings.blackKingSide = false;
    }

    // disable permission if rook is moved.
    if (move.movingPiece.pieceType == WR) {
        if (move.square == 0) {
            // If rook was moved from a1.
            boardSettings.whiteQueenSide = false;
        }
        if (move.square == 7) {
            // If rook was moved from h1.
            boardSettings.whiteKingSide = false;
        }
    } else if (move.movingPiece.pieceType == BR) {
        if (move.square == 56) {
            // If rook was moved from a8.
            boardSettings.blackQueenSide = false;
        }

        if (move.square == 63) {
            // If rook was moved from h8.
            boardSettings.blackKingSide = false;
        }
    }

    // disable permission if rook is captured.
    if (move.capturedPiece.pieceType == WR) {
        if (move.moveSquare == 0) {
            // If a piece moved on a1.
            boardSettings.whiteQueenSide = false;
        }
        if (move.moveSquare == 7) {
            // If a piece moved on h1.
            boardSettings.whiteKingSide = false;
        }
    } else if (move.capturedPiece.pieceType == BR) {
        if (move.moveSquare == 56) {
            // If a piece moved on a8.
            boardSettings.blackQueenSide = false;
        }
        if (move.moveSquare == 63) {
            // If a piece moved on h8.
            boardSettings.blackKingSide = false;
        }
    }
}

bool Board::isCheckMate(bool isWhite) {
    // Count if there are no possible moves anymore.
    int counter = 0;
    for (Move& move: moveGenUtils::getAllPseudoLegalMoves(*this, isWhite)) {
        if (makeMove(move)) {
            counter++;
            popLastMove();
        }
    }
    return counter == 0;
}

Move Board::parseMove(const std::string& input) const {
    char promotion_figure = ' ';

    // Use the ascii code of the char to subtract a number to get the correct number.
    const int x = input[0] - 96;
    const int y = input[1] - 48;
    const int position = calculateSquare(x, y);

    const char figure = board[position].toChar();

    const int move_x = input[2] - 96;
    const int move_y = input[3] - 48;

    const int movePosition = calculateSquare(move_x, move_y);

    if (input.length() == 5) {
        promotion_figure = input[4];
    }

    MoveType moveType = NORMAL;

    // If figure is a king.
    if (figure == 'K') {
        if (position == 4 && (movePosition == 6 || movePosition == 2)) {
            // Apply castling if specific move is parsed.
            moveType = CASTLING;
        }
    }

    if (figure == 'k') {
        if (position == 60 && (movePosition == 62 || movePosition == 58)) {
            // Apply castling if specific move is parsed.
            moveType = CASTLING;
        }
    }
    // If you try to move to a ep square set the move to ep.
    if (movePosition == boardSettings.epSquare) {
        moveType = EN_PASSANT;
    }

    // If the promotion is given set the move to a promotion.
    if (promotion_figure != ' ') {
        moveType = PROMOTION;
        promotion_figure = player == WHITE
                               ? static_cast<char>(std::toupper(promotion_figure))
                               : promotion_figure;
    }

    // Initialize the move with all data.
    return Move{movePosition, position, Piece(figure), board[movePosition], Piece(promotion_figure), moveType};
}

bool Board::tryToMovePiece(const Move& move) {
    bool capture = false;
    if (move.capturedPiece.pieceType != EMPTY) {
        // Detect if there is a capture.
        capture = true;
    }

    // Check if you try to move a piece of the opponent.
    if ((player == BLACK && move.movingPiece.isWhite()) || (player == WHITE && (!move.movingPiece.isWhite()))) {
        return false;
    }

    // Check if moveSquare is out of bounds!
    if (move.square < 0 || move.square > 63 || move.moveSquare < 0 || move.moveSquare > 63) {
        return false;
    }

    // Check if moving piece is really that piece.
    if (board[move.square].pieceType != move.movingPiece.pieceType) {
        return false;
    }

    if (capture) {
        // Check if you try to capture your own team.
        if ((board[move.square].isWhite() && board[move.moveSquare].isWhite()) ||
            ((!board[move.square].isWhite()) && (!board[move.moveSquare].isWhite()))) {
            return false;
        }
    }

    // Check if your move is pseudo legal.
    if (moveGenUtils::getAllPseudoLegalMoves(*this, player == WHITE).contains(move)) {
        // Check moves legality.
        if (!makeMove(move)) {
            std::cout << "Move not legal! Check your king!" << std::endl;
            return false;
        }
        return true;
    }
    return false;
}

void Board::readFen(const std::string& input) {
    std::vector<std::string> fenSettings;

    // Parse all the input in my vector while I cut it.
    std::istringstream iss(input);
    for (std::string s; iss >> s;) fenSettings.push_back(s);

    if (fenSettings.size() != 6) {
        throw InvalidFENException("Invalid length!");
    }

    // Initialize the player and settings.
    player = WHITE;
    boardSettings = board_setting{100, false, false, false, false};

    // Add the pieces to the board.
    int x = 1;
    int y = 8;
    int squares = 0;
    bool whiteKing = false;
    bool blackKing = false;
    for (char& character: fenSettings[0]) {
        // Row change if '/'.
        if (character == '/') {
            y--;
            // Reset x to 1.
            x = 1;
            continue;
        }

        if (std::isdigit(character)) {
            // If it is a digit.
            for (int i = character - '0'; i > 0; --i) {
                // Loop through the squares which will be empty.
                // Set them empty.
                board[calculateSquare(x, y)] = Piece(EMPTY);
                // Increment x cause new square.
                x++;
                squares++;
            }
            // After looping go to the next char.
            continue;
        }

        // Check if kings are present.
        if (character == 'k') {
            if (blackKing) {
                throw InvalidFENException("Your FEN has two black kings!");
            }
            blackKing = true;
        } else if (character == 'K') {
            if (whiteKing) {
                throw InvalidFENException("Your FEN has two white kings!");
            }
            whiteKing = true;
        }

        // Just set on the square that piece.
        board[calculateSquare(x, y)] = Piece(character);
        x++;
        squares++;
    }

    // If one king is missing throw exception.
    if ((!whiteKing) || (!blackKing)) {
        throw InvalidFENException("Your FEN is missing a king!");
    }

    // Check if enough squares.
    if (squares != 64) {
        throw InvalidFENException("Your FEN is missing some squares!");
    }

    // Set turn
    if (fenSettings[1] == "b") {
        player = BLACK;
    }

    // Set casteling permissions to true
    if (fenSettings[2].find('K') != std::string::npos) {
        boardSettings.whiteKingSide = true;
    }
    if (fenSettings[2].find('Q') != std::string::npos) {
        boardSettings.whiteQueenSide = true;
    }
    if (fenSettings[2].find('k') != std::string::npos) {
        boardSettings.blackKingSide = true;
    }
    if (fenSettings[2].find('q') != std::string::npos) {
        boardSettings.blackQueenSide = true;
    }

    // Set ep square if given.
    if (fenSettings[3] != "-") {
        int col = fenSettings[3][0] - 96;
        int row = fenSettings[3][1] - 48;
        boardSettings.epSquare = calculateSquare(col, row);
    }

    // Convert the char to an int and subtract 48 (ascii value).
    // It is not pretty but I don't know a better way.
    boardSettings.lastMovesSincePawnOrCapture = fenSettings[4][0] - 48;
    boardSettings.turns = fenSettings[5][0] - 48;

    // Build the new hash.
    buildHashForBoard();

    // Save current settings.
    history.push_back(boardSettings);
    // Clear the moves.
    moves.clear();
}

void Board::printCurrentBoard() const {
    // Print the current turn.
    if (player == WHITE) {
        std::cout << "Current turn: "
                << "White" << std::endl;
    } else {
        std::cout << "Current turn: "
                << "Black" << std::endl;
    }

    // Print the board.
    for (int y = 8; y >= 1; y--) {
        std::cout << y << " | ";
        for (int x = 1; x <= 8; x++) {
            std::cout << "[" << board[calculateSquare(x, y)].toChar() << "]";
        }
        std::cout << std::endl;
    }
    std::cout << "     a";
    for (int i = 2; i <= 8; i++) {
        std::cout << "  " << static_cast<char>(i + 96);
    }
    std::cout << std::endl;
}

std::string Board::getFen() const {
    std::string outPutFen;
    // Got through y = 8-1 and x = 1-8.
    for (int y = 8; y > 0; y--) {
        // Representing the number of empty fields in one row.
        int emptyFields = 0;
        for (int x = 1; x < 9; x++) {
            Piece piece = board[calculateSquare(x, y)];
            if (piece.pieceType == EMPTY) {
                emptyFields++;
                // If the whole row is Empty fields it should print it after hitting the end.
                if (x == 8) {
                    outPutFen += std::to_string(emptyFields);
                }
            } else {
                // If there were only empty fields until now. Add the number!
                if (emptyFields != 0) {
                    outPutFen += std::to_string(emptyFields);
                    emptyFields = 0;
                }
                outPutFen += board[calculateSquare(x, y)].toChar();
            }
        }
        // Add the line break!
        if (y != 1) {
            outPutFen += '/';
        }
    }

    outPutFen += ' ';
    // Add the turn.
    outPutFen += (player == WHITE) ? "w" : "b";
    outPutFen += ' ';

    // Add castling rights!
    if (boardSettings.blackQueenSide || boardSettings.whiteQueenSide || boardSettings.whiteKingSide ||
        boardSettings.blackKingSide) {
        if (boardSettings.whiteKingSide) {
            outPutFen += 'K';
        }
        if (boardSettings.whiteQueenSide) {
            outPutFen += 'Q';
        }
        if (boardSettings.blackKingSide) {
            outPutFen += 'k';
        }
        if (boardSettings.blackQueenSide) {
            outPutFen += 'q';
        }
    } else {
        outPutFen += '-';
    }

    outPutFen += ' ';
    // Add EP square
    outPutFen += boardSettings.epSquare != 100 ? convertToXandY(boardSettings.epSquare) : "-";
    outPutFen += ' ';
    // Add turns
    outPutFen += std::to_string(boardSettings.lastMovesSincePawnOrCapture);
    outPutFen += ' ';
    outPutFen += std::to_string(boardSettings.turns);
    return outPutFen;
}

void Board::reset() {
    readFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    moves.clear();
}
