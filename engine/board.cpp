//
// Created by Marvin Becker on 15.12.23.
//

#include <iostream>

#include "./board.h"
#include "./chess_bot.h"
#include "utils.h"
#include "./exceptions/board_exception.h"
#include "./exceptions/fen_exception.h"

bool Board::is_king_in_check(bool piece_color) {
    // Go through board.
    for (int y = 8; y >= 1; y--) {
        for (int x = 1; x <= 8; x++) {
            if (board[calculateSquare(x, y)].piece_type == (piece_color ? WK : BK)) {
                // If piece is the correct King.
                // Check if the king square is attacked.
                return is_square_attacked({x, y}, piece_color);
            }
        }
    }

    // If no king is found throw exception.
    throw BoardInterruptException("No king found!");
}

bool Board::is_square_attacked(const std::pair<int, int>& SQUARE, const bool PIECE_COLOR) {
    PseudoLegalMoves allKnightMoves;
    PseudoLegalMoves allPawnMoves;
    PseudoLegalMoves allBishopMoves;
    PseudoLegalMoves allRookMoves;

    // getALlPossible###Moves does only return captures of the opponent pieces. So no need to check again if you capture
    // your own piece

    moveGenUtils::get_all_possible_knight_moves(SQUARE, *this, allKnightMoves, PIECE_COLOR);
    for (Move& move: allKnightMoves) {
        if (move.captured_piece.piece_type == WN || move.captured_piece.piece_type == BN) {
            return true;
        }
    }

    moveGenUtils::get_all_possible_pawn_moves(SQUARE, *this, allPawnMoves, PIECE_COLOR);
    for (Move& move: allPawnMoves) {
        if (move.captured_piece.piece_type == WP || move.captured_piece.piece_type == BP) {
            return true;
        }
    }

    // You also have to check for Queens because they can move like the bishop too!
    moveGenUtils::get_all_possible_bishop_moves(SQUARE, *this, allBishopMoves, PIECE_COLOR);
    for (Move& move: allBishopMoves) {
        if (move.captured_piece.piece_type == WB || move.captured_piece.piece_type == BB ||
            move.captured_piece.piece_type == WQ || move.captured_piece.piece_type == BQ) {
            return true;
        }
    }

    // You also have to check for Queens because they can move like the rook too!
    moveGenUtils::get_all_possible_rook_moves(SQUARE, *this, allRookMoves, PIECE_COLOR);
    for (Move& move: allRookMoves) {
        if (move.captured_piece.piece_type == WR || move.captured_piece.piece_type == BR ||
            move.captured_piece.piece_type == WQ || move.captured_piece.piece_type == BQ) {
            return true;
        }
    }

    // Specifically check if there is a king near you.
    std::pair<int, int> directions[8] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};
    for (const auto& [FST, SND]: directions) {
        const int X = SQUARE.first + FST;
        if (const int Y = SQUARE.second + SND; X > 0 && Y > 0 && X < 9 && Y < 9) {
            if (board[calculateSquare(X, Y)].piece_type == (PIECE_COLOR ? BK : WK)) {
                return true;
            }
        }
    }

    return false;
}

bool Board::pop_last_move() {
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
    board[last_move.move_square] = last_move.captured_piece;
    // Set the moved piece on its original position.
    board[last_move.square] = last_move.moving_piece;

    if (last_move.move_type == EN_PASSANT) {
        // If the move was an EP.
        // Get the square behind the ep square by adding or subtracting 8 (One row).
        int enPassantSquare = last_move.move_square + (last_move.moving_piece.is_white() ? -8 : +8);
        // Set the piece which got caught there.
        board[enPassantSquare].piece_type = (last_move.moving_piece.is_white() ? BP : WP);
    }

    if (last_move.move_type == CASTLING) {
        // If the move was castling.
        if (last_move.move_square == 6) {
            // Check if the King moved to square g1.
            // Reset the rook to h1.
            board[7].piece_type = WR;
            board[5].piece_type = EMPTY;
        }
        if (last_move.move_square == 2) {
            // Check if the King moved to square c1.
            // Reset the rook to a1.
            board[0].piece_type = WR;
            board[3].piece_type = EMPTY;
        }
        if (last_move.move_square == 62) {
            // Check if the King moved to square g8.
            // Reset the rook to h8.
            board[63].piece_type = BR;
            board[61].piece_type = EMPTY;
        }
        if (last_move.move_square == 58) {
            // Check if the King moved to square c8.
            // Reset the rook to h8.
            board[56].piece_type = BR;
            board[59].piece_type = EMPTY;
        }
    }

    // Set player back.
    player = player == WHITE ? BLACK : WHITE;

    // Settings reset.
    board_settings = history.back();

    build_hash_for_board();
    return true;
}

bool Board::make_move(const Move& MOVE) {
    // Set the square to move to the piece where it is currently.
    board[MOVE.move_square] = board[MOVE.square];

    // Set on the old square an EMPTY piece.
    board[MOVE.square].piece_type = EMPTY;

    // Reset the eqSquare.
    board_settings.ep_square = 100;
    // Increment lastMoveSincePawnOrCapture.
    board_settings.last_moves_since_pawn_or_capture++;

    // If move is a promotion set on the future square the promotion piece.
    if (MOVE.move_type == PROMOTION) {
        board[MOVE.move_square] = MOVE.promotion_piece;
    }

    // If move is EP then replace the square in front of the move with empty.
    if (MOVE.move_type == EN_PASSANT) {
        int enPassantSquare = MOVE.move_square + (MOVE.moving_piece.piece_type == WP ? -8 : +8);
        board[enPassantSquare].piece_type = EMPTY;
    }

    // If it is a castling move just set the rook at the correct spot.
    if (MOVE.move_type == CASTLING) {
        if (MOVE.move_square == 6) {
            board[7].piece_type = EMPTY;
            board[5].piece_type = WR;
        }
        if (MOVE.move_square == 2) {
            board[0].piece_type = EMPTY;
            board[3].piece_type = WR;
        }
        if (MOVE.move_square == 62) {
            board[63].piece_type = EMPTY;
            board[61].piece_type = BR;
        }
        if (MOVE.move_square == 58) {
            board[56].piece_type = EMPTY;
            board[59].piece_type = BR;
        }
    }

    if (MOVE.moving_piece.piece_type == WP || MOVE.moving_piece.piece_type == BP) {
        if (std::abs(MOVE.square - MOVE.move_square) == 16) {
            // Set EP square if a pawn moves exact 2 rows.
            board_settings.ep_square = MOVE.move_square + (MOVE.moving_piece.is_white() ? -8 : +8);
        }
    }

    // Set the permissions for castling!
    handle_castling_permissions(MOVE);

    if (player == BLACK) {
        board_settings.turns++;
    }

    if (((MOVE.moving_piece.piece_type == WP) || (MOVE.moving_piece.piece_type == BP)) ||
        MOVE.captured_piece.piece_type != EMPTY) {
        board_settings.last_moves_since_pawn_or_capture = 0;
    }

    // Save move
    moves.push_back(MOVE);
    // Save settings
    history.push_back(board_settings);
    // Reset the player.
    player = player == WHITE ? BLACK : WHITE;

    // Check if your king is in check after the move and pop if yes.
    if (is_king_in_check(player != WHITE)) {
        pop_last_move();
        return false;
    }

    build_hash_for_board();

    // return true if everything is fine.
    return true;
}

void Board::handle_castling_permissions(const Move& MOVE) {
    // If king is moved. disable everything.
    if (MOVE.moving_piece.piece_type == WK) {
        board_settings.white_queen_side = false;
        board_settings.white_king_side = false;
    } else if (MOVE.moving_piece.piece_type == BK) {
        board_settings.black_queen_side = false;
        board_settings.black_king_side = false;
    }

    // disable permission if rook is moved.
    if (MOVE.moving_piece.piece_type == WR) {
        if (MOVE.square == 0) {
            // If rook was moved from a1.
            board_settings.white_queen_side = false;
        }
        if (MOVE.square == 7) {
            // If rook was moved from h1.
            board_settings.white_king_side = false;
        }
    } else if (MOVE.moving_piece.piece_type == BR) {
        if (MOVE.square == 56) {
            // If rook was moved from a8.
            board_settings.black_queen_side = false;
        }

        if (MOVE.square == 63) {
            // If rook was moved from h8.
            board_settings.black_king_side = false;
        }
    }

    // disable permission if rook is captured.
    if (MOVE.captured_piece.piece_type == WR) {
        if (MOVE.move_square == 0) {
            // If a piece moved on a1.
            board_settings.white_queen_side = false;
        }
        if (MOVE.move_square == 7) {
            // If a piece moved on h1.
            board_settings.white_king_side = false;
        }
    } else if (MOVE.captured_piece.piece_type == BR) {
        if (MOVE.move_square == 56) {
            // If a piece moved on a8.
            board_settings.black_queen_side = false;
        }
        if (MOVE.move_square == 63) {
            // If a piece moved on h8.
            board_settings.black_king_side = false;
        }
    }
}

bool Board::is_check_mate(bool isWhite) {
    // Count if there are no possible moves anymore.
    int counter = 0;
    for (Move& move: moveGenUtils::get_all_pseudo_legal_moves(*this, isWhite)) {
        if (make_move(move)) {
            counter++;
            pop_last_move();
        }
    }
    return counter == 0;
}

Move Board::parse_move(const std::string& INPUT) const {
    char promotion_figure = ' ';

    // Use the ascii code of the char to subtract a number to get the correct number.
    const int x = INPUT[0] - 96;
    const int y = INPUT[1] - 48;
    const int position = calculateSquare(x, y);

    const char figure = board[position].to_char();

    const int move_x = INPUT[2] - 96;
    const int move_y = INPUT[3] - 48;

    const int movePosition = calculateSquare(move_x, move_y);

    if (INPUT.length() == 5) {
        promotion_figure = INPUT[4];
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
    if (movePosition == board_settings.ep_square) {
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

bool Board::try_to_move_piece(const Move& MOVE) {
    bool capture = false;
    if (MOVE.captured_piece.piece_type != EMPTY) {
        // Detect if there is a capture.
        capture = true;
    }

    // Check if you try to move a piece of the opponent.
    if ((player == BLACK && MOVE.moving_piece.is_white()) || (player == WHITE && (!MOVE.moving_piece.is_white()))) {
        return false;
    }

    // Check if moveSquare is out of bounds!
    if (MOVE.square < 0 || MOVE.square > 63 || MOVE.move_square < 0 || MOVE.move_square > 63) {
        return false;
    }

    // Check if moving piece is really that piece.
    if (board[MOVE.square].piece_type != MOVE.moving_piece.piece_type) {
        return false;
    }

    if (capture) {
        // Check if you try to capture your own team.
        if ((board[MOVE.square].is_white() && board[MOVE.move_square].is_white()) ||
            ((!board[MOVE.square].is_white()) && (!board[MOVE.move_square].is_white()))) {
            return false;
        }
    }

    // Check if your move is pseudo legal.
    if (moveGenUtils::get_all_pseudo_legal_moves(*this, player == WHITE).contains(MOVE)) {
        // Check moves legality.
        if (!make_move(MOVE)) {
            std::cout << "Move not legal! Check your king!" << std::endl;
            return false;
        }
        return true;
    }
    return false;
}

void Board::read_fen(const std::string& INPUT) {
    std::vector<std::string> fenSettings;

    // Parse all the input in my vector while I cut it.
    std::istringstream iss(INPUT);
    for (std::string s; iss >> s;) fenSettings.push_back(s);

    if (fenSettings.size() != 6) {
        throw InvalidFENException("Invalid length!");
    }

    // Initialize the player and settings.
    player = WHITE;
    board_settings = board_setting{100, false, false, false, false};

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
        board_settings.white_king_side = true;
    }
    if (fenSettings[2].find('Q') != std::string::npos) {
        board_settings.white_queen_side = true;
    }
    if (fenSettings[2].find('k') != std::string::npos) {
        board_settings.black_king_side = true;
    }
    if (fenSettings[2].find('q') != std::string::npos) {
        board_settings.black_queen_side = true;
    }

    // Set ep square if given.
    if (fenSettings[3] != "-") {
        int col = fenSettings[3][0] - 96;
        int row = fenSettings[3][1] - 48;
        board_settings.ep_square = calculateSquare(col, row);
    }

    // Convert the char to an int and subtract 48 (ascii value).
    // It is not pretty but I don't know a better way.
    board_settings.last_moves_since_pawn_or_capture = fenSettings[4][0] - 48;
    board_settings.turns = fenSettings[5][0] - 48;

    // Build the new hash.
    build_hash_for_board();

    // Save current settings.
    history.push_back(board_settings);
    // Clear the moves.
    moves.clear();
}

void Board::print_current_board() const {
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
            std::cout << "[" << board[calculateSquare(x, y)].to_char() << "]";
        }
        std::cout << std::endl;
    }
    std::cout << "     a";
    for (int i = 2; i <= 8; i++) {
        std::cout << "  " << static_cast<char>(i + 96);
    }
    std::cout << std::endl;
}

std::string Board::get_fen() const {
    std::string outPutFen;
    // Got through y = 8-1 and x = 1-8.
    for (int y = 8; y > 0; y--) {
        // Representing the number of empty fields in one row.
        int emptyFields = 0;
        for (int x = 1; x < 9; x++) {
            Piece piece = board[calculateSquare(x, y)];
            if (piece.piece_type == EMPTY) {
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
                outPutFen += board[calculateSquare(x, y)].to_char();
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
    if (board_settings.black_queen_side || board_settings.white_queen_side || board_settings.white_king_side ||
        board_settings.black_king_side) {
        if (board_settings.white_king_side) {
            outPutFen += 'K';
        }
        if (board_settings.white_queen_side) {
            outPutFen += 'Q';
        }
        if (board_settings.black_king_side) {
            outPutFen += 'k';
        }
        if (board_settings.black_queen_side) {
            outPutFen += 'q';
        }
    } else {
        outPutFen += '-';
    }

    outPutFen += ' ';
    // Add EP square
    outPutFen += board_settings.ep_square != 100 ? convert_to_x_and_y(board_settings.ep_square) : "-";
    outPutFen += ' ';
    // Add turns
    outPutFen += std::to_string(board_settings.last_moves_since_pawn_or_capture);
    outPutFen += ' ';
    outPutFen += std::to_string(board_settings.turns);
    return outPutFen;
}

void Board::reset() {
    read_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    moves.clear();
}
