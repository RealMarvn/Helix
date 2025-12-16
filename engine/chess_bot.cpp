#include "./chess_bot.h"

#include <iostream>

void ChessBot::reset_tt()
{
    tt_.clear();
}

int ChessBot::eval(Board& board)
{
    int mg[2] = {};
    int eg[2] = {};
    int gamePhase = 0;
    for (int i = 0; i < 64; i++)
    {
        Piece piece = board[i];
        // Calculate game phase for 'Tampered Eval'.
        gamePhase += piece.get_game_phase_value();
        // Add PSQT values + material value.
        switch (piece.piece_type)
        {
        case WP:
            mg[0] += mg_pawn_table[i ^ 56] + piece.get_material_value(false);
            eg[0] += eg_pawn_table[i ^ 56] + piece.get_material_value(true);
            break;
        case WN:
            mg[0] += mg_knight_table[i ^ 56] + piece.get_material_value(false);
            eg[0] += eg_knight_table[i ^ 56] + piece.get_material_value(true);
            break;
        case WB:
            mg[0] += mg_bishop_table[i ^ 56] + piece.get_material_value(false);
            eg[0] += eg_bishop_table[i ^ 56] + piece.get_material_value(true);
            break;
        case WR:
            mg[0] += mg_rook_table[i ^ 56] + piece.get_material_value(false);
            eg[0] += eg_rook_table[i ^ 56] + piece.get_material_value(true);
            break;
        case WQ:
            mg[0] += mg_queen_table[i ^ 56] + piece.get_material_value(false);
            eg[0] += eg_queen_table[i ^ 56] + piece.get_material_value(true);
            break;
        case WK:
            mg[0] += mg_king_table[i ^ 56] + piece.get_material_value(false);
            eg[0] += eg_king_table[i ^ 56] + piece.get_material_value(true);
            break;
        case BP:
            mg[1] += mg_pawn_table[i] + piece.get_material_value(false);
            eg[1] += eg_pawn_table[i] + piece.get_material_value(true);
            break;
        case BN:
            mg[1] += mg_knight_table[i] + piece.get_material_value(false);
            eg[1] += eg_knight_table[i] + piece.get_material_value(true);
            break;
        case BB:
            mg[1] += mg_bishop_table[i] + piece.get_material_value(false);
            eg[1] += eg_bishop_table[i] + piece.get_material_value(true);
            break;
        case BR:
            mg[1] += mg_rook_table[i] + piece.get_material_value(false);
            eg[1] += eg_rook_table[i] + piece.get_material_value(true);
            break;
        case BQ:
            mg[1] += mg_queen_table[i] + piece.get_material_value(false);
            eg[1] += eg_queen_table[i] + piece.get_material_value(true);
            break;
        case BK:
            mg[1] += mg_king_table[i] + piece.get_material_value(false);
            eg[1] += eg_king_table[i] + piece.get_material_value(true);
            break;
        case EMPTY:
            break;
        }
    }

    // Calculate PSQT and game phase with tampered eval
    // (https://www.chessprogramming.org/PeSTO's_Evaluation_Function) (MODIFIED by
    // MARVIN).
    const int MG_SCORE = mg[board.player != WHITE] - mg[board.player == WHITE];
    const int EG_SCORE = eg[board.player != WHITE] - eg[board.player == WHITE];
    // Calculate the game phase.
    int mgPhase = gamePhase;
    if (mgPhase > 24)
        mgPhase = 24;
    const int EG_PHASE = 24 - mgPhase;

    const int EVALUATION = (((MG_SCORE * mgPhase) + (EG_SCORE * EG_PHASE)) / 24);

    // Give a bonus to side to move (TEMPO).
    return (EVALUATION + 20);
}

Move ChessBot::generate_best_next_move(Board& board, const int time_constraint)
{
    // Set the time to now.
    iterative_time_point = std::chrono::high_resolution_clock::now();
    iterative_time_constraint = time_constraint;
    tt_.new_search();
    Move bestMove{};
    // Run until the timeout returns true.
    for (int i = 1;; i++)
    {
        // Search the best move for depth i.
        const Move move = search_best_next_move(board, i);
        if (is_time_up())
        {
            // If the time is up, break the loop and don't apply the not fully
            // evaluated move.
            break;
        }
        // Set the move if the search is fully done.
        bestMove = move;
    }
    // Return the best move calculated at depth i.
    return bestMove;
}

Move ChessBot::generate_best_next_move_fixed_depth(Board& board, const int DEPTH)
{
    // Timelimit to inf, because we now limit with DEPTH.
    iterative_time_constraint = std::numeric_limits<int>::max();
    iterative_time_point = std::chrono::high_resolution_clock::now();
    tt_.new_search();

    // Search to specific depth.
    return search_best_next_move(board, DEPTH);
}

int ChessBot::search(Board& board, const int DEPTH, int alpha, const int BETA, const int PLY,
                     Move& best_move)
{
    // Leaf node → Quiescence
    if (DEPTH <= 0)
        return quiescence_search(board, alpha, BETA);

    // Global time limit check
    if (is_time_up())
        return -INT_MAX;

    const int ORIGINAL_ALPHA = alpha;
    const std::uint64_t key = board.get_hash();

    // Transposition table probe (may return exact score or safe cutoff)
    Move tt_move{};
    if (int tt_score = 0; tt_.probe(key, DEPTH, alpha, BETA, PLY, tt_score, tt_move))
    {
        return tt_score;
    }

    // Get all possible moves.
    auto moveList = moveGenUtils::get_all_pseudo_legal_moves(board, board.player == WHITE);
    // Sort so the best moves are first (TT move first, then MVV-LVA).
    moveList.sort_move_list_mvv_lva(tt_move);

    int legalMoves = 0;

    // First best score should be the worst.
    int bestScore = -INT_MAX;
    Move localBestMove{};

    for (Move& move : moveList)
    {
        int score;

        // Make every move and gather the value of the opponent.
        if (board.make_move(move))
        {
            score = -search(board, DEPTH - 1, -BETA, -alpha, PLY + 1, best_move);
            legalMoves++;
            board.pop_last_move();
        }
        else
        {
            continue;
        }

        // Set best score if the current one is less.
        if (score > bestScore)
        {
            bestScore = score;
            localBestMove = move;
            if (PLY == 0)
            {
                // Set best move if it is the root.
                best_move = move;
            }
        }

        // ALPHA BETA PRUNING
        if (bestScore > alpha)
        {
            alpha = bestScore;
        }

        if (alpha >= BETA)
        {
            break; // beta kill
        }
    }

    // If no legal moves
    if (legalMoves == 0)
    {
        if (board.is_king_in_check(board.player == WHITE))
        {
            return -INT_MAX + PLY; // checkmate
        }
        else
        {
            return 0; // stalemate
        }
    }

    // Store to transposition table
    auto flag = TTFlag::Exact;
    if (bestScore <= ORIGINAL_ALPHA)
        flag = TTFlag::UpperBound;
    else if (bestScore >= BETA)
        flag = TTFlag::LowerBound;

    tt_.store(key, DEPTH, bestScore, flag, PLY, localBestMove);

    return bestScore;
}

int ChessBot::quiescence_search(Board& board, int alpha, const int BETA)
{
    // Eval the position.
    const int STAND_PAT = eval(board);

    if (STAND_PAT >= BETA)
    {
        return BETA;
    }
    if (alpha < STAND_PAT)
    {
        alpha = STAND_PAT;
    }

    // Get all possible moves.
    Move tt_move{};
    tt_.probe_move(board.get_hash(), tt_move);
    auto moveList = moveGenUtils::get_all_pseudo_legal_moves(board, board.player == WHITE);
    moveList.sort_move_list_mvv_lva(tt_move);

    // First best score should be the worst.
    int bestScore = STAND_PAT;

    for (Move& move : moveList)
    {
        int score;

        // Filter the normal moves so I only have captures.
        if (move.captured_piece.piece_type == EMPTY && move.move_type != EN_PASSANT)
        {
            continue;
        }

        // Make every move and gather the value of the opponent.
        if (board.make_move(move))
        {
            score = -quiescence_search(board, -BETA, -alpha);
            board.pop_last_move();
        }
        else
        {
            continue;
        }

        // Update best score.
        bestScore = std::max(bestScore, score);
        // if score is less than alpha continue.
        if (score <= alpha)
            continue;
        // set the score
        alpha = score;
        if (score >= BETA)
        {
            break;
        }
    }

    return bestScore;
}

Move ChessBot::search_best_next_move(Board& board, const int DEPTH)
{
    Move move;
    search(board, DEPTH, -INT_MAX, INT_MAX, 0, move);
    return move;
}
