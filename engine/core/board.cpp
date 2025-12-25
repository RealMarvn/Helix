//
// Created by Marvin Becker on 15.12.23.
//

#include <iostream>

#include "../exceptions/board_exception.h"
#include "../exceptions/fen_exception.h"
#include "../search/search.h"
#include "../utils.h"
#include "board.h"

bool Board::is_king_in_check(const bool piece_color)
{
    // Go through board.
    for (int y = 8; y >= 1; y--)
    {
        for (int x = 1; x <= 8; x++)
        {
            // If piece is the correct King.
            // Check if the king square is attacked.
            if (board_[calculateSquare(x, y)].piece_type_ == (piece_color ? WK : BK))
                return is_square_attacked({x, y}, piece_color);
        }
    }

    // If no king is found throw exception.
    throw BoardInterruptException("No king found!");
}

bool Board::is_square_attacked(const std::pair<int, int>& square, const bool piece_color)
{
    PseudoLegalMoves allKnightMoves;
    PseudoLegalMoves allPawnMoves;
    PseudoLegalMoves allBishopMoves;
    PseudoLegalMoves allRookMoves;

    // getALlPossible###Moves does only return captures of the opponent pieces. So
    // no need to check again if you capture your own piece

    moveGenUtils::get_all_possible_knight_moves(square, *this, allKnightMoves, piece_color);
    for (const Move& MOVE : allKnightMoves)
    {
        if (MOVE.captured_piece_.piece_type_ == WN || MOVE.captured_piece_.piece_type_ == BN)
            return true;
    }

    moveGenUtils::get_all_possible_pawn_moves(square, *this, allPawnMoves, piece_color);
    for (const Move& MOVE : allPawnMoves)
    {
        if (MOVE.captured_piece_.piece_type_ == WP || MOVE.captured_piece_.piece_type_ == BP)
            return true;
    }

    // You also have to check for Queens because they can move like the bishop
    // too!
    moveGenUtils::get_all_possible_bishop_moves(square, *this, allBishopMoves, piece_color);
    for (const Move& MOVE : allBishopMoves)
    {
        if (MOVE.captured_piece_.piece_type_ == WB || MOVE.captured_piece_.piece_type_ == BB ||
            MOVE.captured_piece_.piece_type_ == WQ || MOVE.captured_piece_.piece_type_ == BQ)
            return true;
    }

    // You also have to check for Queens because they can move like the rook too!
    moveGenUtils::get_all_possible_rook_moves(square, *this, allRookMoves, piece_color);
    for (const Move& MOVE : allRookMoves)
    {
        if (MOVE.captured_piece_.piece_type_ == WR || MOVE.captured_piece_.piece_type_ == BR ||
            MOVE.captured_piece_.piece_type_ == WQ || MOVE.captured_piece_.piece_type_ == BQ)
            return true;
    }

    // Specifically check if there is a king near you.
    std::pair<int, int> directions[8] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1},
                                         {0, 1},   {1, -1}, {1, 0},  {1, 1}};
    for (const auto& [FST, SND] : directions)
    {
        const int X = square.first + FST;
        if (const int Y = square.second + SND; X > 0 && Y > 0 && X < 9 && Y < 9)
        {
            if (board_[calculateSquare(X, Y)].piece_type_ == (piece_color ? BK : WK))
                return true;
        }
    }

    return false;
}

bool Board::pop_last_move()
{
    // You shouldn't be able to pop if there is nothing.
    if (moves_.empty() || history_.empty())
        return false;

    // Save last move.
    const Move LAST_MOVE = moves_.back();
    // Pop it.
    moves_.pop_back();
    // Pop the history.
    history_.pop_back();
    // Set the piece which got caught on the moved square.
    board_[LAST_MOVE.move_square_] = LAST_MOVE.captured_piece_;
    // Set the moved piece on its original position.
    board_[LAST_MOVE.square_] = LAST_MOVE.moving_piece_;

    if (LAST_MOVE.move_type_ == EN_PASSANT)
    {
        // If the move was an EP.
        // Get the square behind the ep square by adding or subtracting 8 (One row).
        const int EN_PASSANT_SQUARE =
            LAST_MOVE.move_square_ + (LAST_MOVE.moving_piece_.is_white() ? -8 : +8);
        // Set the piece which got caught there.
        board_[EN_PASSANT_SQUARE].piece_type_ = (LAST_MOVE.moving_piece_.is_white() ? BP : WP);
    }

    if (LAST_MOVE.move_type_ == CASTLING)
    {
        // If the move was castling.
        if (LAST_MOVE.move_square_ == 6)
        {
            // Check if the King moved to square g1.
            // Reset the rook to h1.
            board_[7].piece_type_ = WR;
            board_[5].piece_type_ = EMPTY;
        }
        if (LAST_MOVE.move_square_ == 2)
        {
            // Check if the King moved to square c1.
            // Reset the rook to a1.
            board_[0].piece_type_ = WR;
            board_[3].piece_type_ = EMPTY;
        }
        if (LAST_MOVE.move_square_ == 62)
        {
            // Check if the King moved to square g8.
            // Reset the rook to h8.
            board_[63].piece_type_ = BR;
            board_[61].piece_type_ = EMPTY;
        }
        if (LAST_MOVE.move_square_ == 58)
        {
            // Check if the King moved to square c8.
            // Reset the rook to h8.
            board_[56].piece_type_ = BR;
            board_[59].piece_type_ = EMPTY;
        }
    }

    // Set player back.
    player_ = player_ == WHITE ? BLACK : WHITE;

    // Settings reset.
    board_settings_ = history_.back();

    build_hash_for_board();
    return true;
}

bool Board::make_move(const Move& move)
{
    // Set the square to move to the piece where it is currently.
    board_[move.move_square_] = board_[move.square_];

    // Set on the old square an EMPTY piece.
    board_[move.square_].piece_type_ = EMPTY;

    // Reset the eqSquare.
    board_settings_.ep_square_ = 100;
    // Increment lastMoveSincePawnOrCapture.
    board_settings_.last_moves_since_pawn_or_capture_++;

    // If move is a promotion set on the future square the promotion piece.
    if (move.move_type_ == PROMOTION)
        board_[move.move_square_] = move.promotion_piece_;

    // If move is EP then replace the square in front of the move with empty.
    if (move.move_type_ == EN_PASSANT)
    {
        const int EN_PASSANT_SQUARE =
            move.move_square_ + (move.moving_piece_.piece_type_ == WP ? -8 : +8);

        board_[EN_PASSANT_SQUARE].piece_type_ = EMPTY;
    }

    // If it is a castling move just set the rook at the correct spot.
    if (move.move_type_ == CASTLING)
    {
        if (move.move_square_ == 6)
        {
            board_[7].piece_type_ = EMPTY;
            board_[5].piece_type_ = WR;
        }
        if (move.move_square_ == 2)
        {
            board_[0].piece_type_ = EMPTY;
            board_[3].piece_type_ = WR;
        }
        if (move.move_square_ == 62)
        {
            board_[63].piece_type_ = EMPTY;
            board_[61].piece_type_ = BR;
        }
        if (move.move_square_ == 58)
        {
            board_[56].piece_type_ = EMPTY;
            board_[59].piece_type_ = BR;
        }
    }

    if (move.moving_piece_.piece_type_ == WP || move.moving_piece_.piece_type_ == BP)
    {
        // Set EP square if a pawn moves exact 2 rows.
        if (std::abs(move.square_ - move.move_square_) == 16)
            board_settings_.ep_square_ =
                move.move_square_ + (move.moving_piece_.is_white() ? -8 : +8);
    }

    // Set the permissions for castling!
    handle_castling_permissions(move);

    if (player_ == BLACK)
        board_settings_.turns_++;

    if (((move.moving_piece_.piece_type_ == WP) || (move.moving_piece_.piece_type_ == BP)) ||
        move.captured_piece_.piece_type_ != EMPTY)
        board_settings_.last_moves_since_pawn_or_capture_ = 0;

    // Save move
    moves_.push_back(move);
    // Save settings
    history_.push_back(board_settings_);
    // Reset the player.
    player_ = player_ == WHITE ? BLACK : WHITE;

    // Check if your king is in check after the move and pop if yes.
    if (is_king_in_check(player_ != WHITE))
    {
        pop_last_move();
        return false;
    }

    build_hash_for_board();

    // return true if everything is fine.
    return true;
}

void Board::handle_castling_permissions(const Move& move)
{
    // If king is moved. disable everything.
    if (move.moving_piece_.piece_type_ == WK)
    {
        board_settings_.white_queen_side_ = false;
        board_settings_.white_king_side_ = false;
    }
    else if (move.moving_piece_.piece_type_ == BK)
    {
        board_settings_.black_queen_side_ = false;
        board_settings_.black_king_side_ = false;
    }

    // disable permission if rook is moved.
    if (move.moving_piece_.piece_type_ == WR)
    {
        // If rook was moved from a1.
        if (move.square_ == 0)
            board_settings_.white_queen_side_ = false;

        // If rook was moved from h1.
        if (move.square_ == 7)
            board_settings_.white_king_side_ = false;
    }
    else if (move.moving_piece_.piece_type_ == BR)
    {
        // If rook was moved from a8.
        if (move.square_ == 56)
            board_settings_.black_queen_side_ = false;

        // If rook was moved from h8.
        if (move.square_ == 63)
            board_settings_.black_king_side_ = false;
    }

    // disable permission if rook is captured.
    if (move.captured_piece_.piece_type_ == WR)
    {
        // If a piece moved on a1.
        if (move.move_square_ == 0)
            board_settings_.white_queen_side_ = false;

        // If a piece moved on h1.
        if (move.move_square_ == 7)
            board_settings_.white_king_side_ = false;
    }
    else if (move.captured_piece_.piece_type_ == BR)
    {
        // If a piece moved on a8.
        if (move.move_square_ == 56)
            board_settings_.black_queen_side_ = false;

        // If a piece moved on h8.
        if (move.move_square_ == 63)
            board_settings_.black_king_side_ = false;
    }
}

bool Board::is_check_mate(const bool is_white)
{
    // Count if there are no possible moves anymore.
    int counter = 0;
    for (Move& move : moveGenUtils::get_all_pseudo_legal_moves(*this, is_white))
    {
        if (make_move(move))
        {
            counter++;
            pop_last_move();
        }
    }
    return counter == 0;
}

Move Board::parse_move(const std::string& input) const
{
    char promotion_figure = ' ';

    // Use the ascii code of the char to subtract a number to get the correct
    // number.
    const int x = input[0] - 96;
    const int y = input[1] - 48;
    const int square = calculateSquare(x, y);

    const int move_x = input[2] - 96;
    const int move_y = input[3] - 48;

    const int move_square = calculateSquare(move_x, move_y);

    // Check if the squares are even in valid range.
    if (square < 0 || square > 63 || move_square < 0 || move_square > 63)
        return Move{};

    const char figure = board_[square].to_char();

    if (input.length() == 5)
        promotion_figure = input[4];

    MoveType moveType = NORMAL;

    // If figure is a king.
    if (figure == 'K')
    {
        // Apply castling if specific move is parsed.
        if (square == 4 && (move_square == 6 || move_square == 2))
            moveType = CASTLING;
    }

    if (figure == 'k')
    {
        // Apply castling if specific move is parsed.
        if (square == 60 && (move_square == 62 || move_square == 58))
            moveType = CASTLING;
    }

    // If you try to move to a ep square set the move to ep.
    if (move_square == board_settings_.ep_square_)
        moveType = EN_PASSANT;

    // If the promotion is given set the move to a promotion.
    if (promotion_figure != ' ')
    {
        moveType = PROMOTION;
        promotion_figure =
            player_ == WHITE ? static_cast<char>(std::toupper(promotion_figure)) : promotion_figure;
    }

    // Initialize the move with all data.
    return Move{move_square, square, Piece(figure), board_[move_square], Piece(promotion_figure),
                moveType};
}

bool Board::try_to_move_piece(const Move& move)
{
    bool capture = false;
    // Detect if there is a capture.
    if (move.captured_piece_.piece_type_ != EMPTY)
        capture = true;

    // Check if you try to move a piece of the opponent.
    if ((player_ == BLACK && move.moving_piece_.is_white()) ||
        (player_ == WHITE && (!move.moving_piece_.is_white())))
        return false;

    // Check if moveSquare is out of bounds!
    if (move.square_ < 0 || move.square_ > 63 || move.move_square_ < 0 || move.move_square_ > 63)
        return false;

    // Check if moving piece is really that piece.
    if (board_[move.square_].piece_type_ != move.moving_piece_.piece_type_)
        return false;

    if (capture)
    {
        // Check if you try to capture your own team.
        if ((board_[move.square_].is_white() && board_[move.move_square_].is_white()) ||
            ((!board_[move.square_].is_white()) && (!board_[move.move_square_].is_white())))
            return false;
    }

    // Check if your move is pseudo legal.
    if (moveGenUtils::get_all_pseudo_legal_moves(*this, player_ == WHITE).contains(move))
    {
        // Check moves legality.
        if (!make_move(move))
        {
            std::cout << "Move not legal! Check your king!" << std::endl;
            return false;
        }
        return true;
    }
    return false;
}

void Board::read_fen(const std::string& input)
{
    std::vector<std::string> fenSettings;

    // Parse all the input in my vector while I cut it.
    std::istringstream iss(input);
    for (std::string s; iss >> s;)
        fenSettings.push_back(s);

    if (fenSettings.size() != 6)
        throw InvalidFENException("Invalid length!");

    // Initialize the player and settings.
    player_ = WHITE;
    board_settings_ = board_setting{100, false, false, false, false};

    // Add the pieces to the board.
    int x = 1;
    int y = 8;
    int squares = 0;
    bool whiteKing = false;
    bool blackKing = false;
    for (const char& CHARACTER : fenSettings[0])
    {
        // Row change if '/'.
        if (CHARACTER == '/')
        {
            y--;
            // Reset x to 1.
            x = 1;
            continue;
        }

        if (std::isdigit(CHARACTER))
        {
            // If it is a digit.
            for (int i = CHARACTER - '0'; i > 0; --i)
            {
                // Loop through the squares which will be empty.
                // Set them empty.
                board_[calculateSquare(x, y)] = Piece(EMPTY);
                // Increment x cause new square.
                x++;
                squares++;
            }
            // After looping go to the next char.
            continue;
        }

        // Check if kings are present.
        if (CHARACTER == 'k')
        {
            if (blackKing)
                throw InvalidFENException("Your FEN has two black kings!");

            blackKing = true;
        }
        else if (CHARACTER == 'K')
        {
            if (whiteKing)
                throw InvalidFENException("Your FEN has two white kings!");

            whiteKing = true;
        }

        // Just set on the square that piece.
        board_[calculateSquare(x, y)] = Piece(CHARACTER);
        x++;
        squares++;
    }

    // If one king is missing throw exception.
    if ((!whiteKing) || (!blackKing))
        throw InvalidFENException("Your FEN is missing a king!");

    // Check if enough squares.
    if (squares != 64)
        throw InvalidFENException("Your FEN is missing some squares!");

    // Set turn
    if (fenSettings[1] == "b")
        player_ = BLACK;

    // Set casteling permissions to true
    if (fenSettings[2].find('K') != std::string::npos)
        board_settings_.white_king_side_ = true;

    if (fenSettings[2].find('Q') != std::string::npos)
        board_settings_.white_queen_side_ = true;

    if (fenSettings[2].find('k') != std::string::npos)
        board_settings_.black_king_side_ = true;

    if (fenSettings[2].find('q') != std::string::npos)
        board_settings_.black_queen_side_ = true;

    // Set ep square if given.
    if (fenSettings[3] != "-")
    {
        const int COL = fenSettings[3][0] - 96;
        const int ROW = fenSettings[3][1] - 48;
        board_settings_.ep_square_ = calculateSquare(COL, ROW);
    }

    // Convert the char to an int and subtract 48 (ascii value).
    // It is not pretty but I don't know a better way.
    board_settings_.last_moves_since_pawn_or_capture_ = fenSettings[4][0] - 48;
    board_settings_.turns_ = fenSettings[5][0] - 48;

    // Build the new hash.
    build_hash_for_board();

    // Save current settings.
    history_.push_back(board_settings_);
    // Clear the moves.
    moves_.clear();
}

void Board::print_current_board() const
{
    // Print the current turn.
    std::cout << "Current turn: " << (player_ == WHITE ? "White" : "Black") << std::endl;

    // Print the board.
    for (int y = 8; y >= 1; y--)
    {
        std::cout << y << " | ";
        for (int x = 1; x <= 8; x++)
        {
            std::cout << "[" << board_[calculateSquare(x, y)].to_char() << "]";
        }
        std::cout << std::endl;
    }
    std::cout << "     a";
    for (int i = 2; i <= 8; i++)
    {
        std::cout << "  " << static_cast<char>(i + 96);
    }
    std::cout << std::endl;
}

std::string Board::get_fen() const
{
    std::string outPutFen;
    // Got through y = 8-1 and x = 1-8.
    for (int y = 8; y > 0; y--)
    {
        // Representing the number of empty fields in one row.
        int emptyFields = 0;
        for (int x = 1; x < 9; x++)
        {
            if (Piece piece = board_[calculateSquare(x, y)]; piece.piece_type_ == EMPTY)
            {
                emptyFields++;
                // If the whole row is Empty fields it should print it after hitting the
                // end.
                if (x == 8)
                    outPutFen += std::to_string(emptyFields);
            }
            else
            {
                // If there were only empty fields until now. Add the number!
                if (emptyFields != 0)
                {
                    outPutFen += std::to_string(emptyFields);
                    emptyFields = 0;
                }
                outPutFen += board_[calculateSquare(x, y)].to_char();
            }
        }
        // Add the line break!
        if (y != 1)
            outPutFen += '/';
    }

    outPutFen += ' ';
    // Add the turn.
    outPutFen += (player_ == WHITE) ? "w" : "b";
    outPutFen += ' ';

    // Add castling rights!
    if (board_settings_.black_queen_side_ || board_settings_.white_queen_side_ ||
        board_settings_.white_king_side_ || board_settings_.black_king_side_)
    {
        if (board_settings_.white_king_side_)
            outPutFen += 'K';

        if (board_settings_.white_queen_side_)
            outPutFen += 'Q';

        if (board_settings_.black_king_side_)
            outPutFen += 'k';

        if (board_settings_.black_queen_side_)
            outPutFen += 'q';
    }
    else
        outPutFen += '-';

    outPutFen += ' ';
    // Add EP square
    outPutFen +=
        board_settings_.ep_square_ != 100 ? convert_to_x_and_y(board_settings_.ep_square_) : "-";
    outPutFen += ' ';
    // Add turns
    outPutFen += std::to_string(board_settings_.last_moves_since_pawn_or_capture_);
    outPutFen += ' ';
    outPutFen += std::to_string(board_settings_.turns_);
    return outPutFen;
}

void Board::reset()
{
    read_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    moves_.clear();
}
