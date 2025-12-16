#include "chess_bot.h"

#include "./eval/eval.h"

#include <iostream>

void ChessBot::reset_tt()
{
    tt.clear();
}

Move ChessBot::generate_best_next_move(Board& board, const int time_constraint)
{
    // Set the time to now.
    iterative_time_point = std::chrono::high_resolution_clock::now();
    iterative_time_constraint = time_constraint;
    tt.new_search();
    Move bestMove{};
    // Run until the timeout returns true.
    for (int i = 1;; i++)
    {
        // Search the best move for depth i.
        const Move move = search_next_move(board, i);
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
    tt.new_search();

    // Search to specific depth.
    return search_next_move(board, DEPTH);
}

int ChessBot::search(Board& board, const int DEPTH, int alpha, const int BETA, const int PLY,
                     Move& best_move)
{
    // Leaf node → Quiescence
    if (DEPTH <= 0)
        return quiescence_search(board, alpha, BETA);

    // Global time limit check
    if (is_time_up())
        return -tt_score_constants::kInfinity;

    const int ORIGINAL_ALPHA = alpha;
    const std::uint64_t key = board.get_hash();

    // Transposition table probe (may return exact score or safe cutoff)
    Move tt_move{};
    if (int tt_score = 0; tt.probe(key, DEPTH, alpha, BETA, PLY, tt_score, tt_move))
    {
        return tt_score;
    }

    // Get all possible moves.
    auto moveList = moveGenUtils::get_all_pseudo_legal_moves(board, board.player == WHITE);
    // Sort so the best moves are first (TT move first, then MVV-LVA).
    moveList.sort_move_list_mvv_lva(tt_move);

    int legalMoves = 0;

    // First best score should be the worst.
    int bestScore = -tt_score_constants::kInfinity;
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
            return -tt_score_constants::kMate + PLY; // checkmate
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

    tt.store(key, DEPTH, bestScore, flag, PLY, localBestMove);

    return bestScore;
}

int ChessBot::quiescence_search(Board& board, int alpha, const int BETA)
{
    // Eval the position.
    const int STAND_PAT = eval::evaluate(board);

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
    tt.probe_move(board.get_hash(), tt_move);
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

Move ChessBot::search_next_move(Board& board, const int DEPTH)
{
    Move move;
    search(board, DEPTH, -tt_score_constants::kInfinity, tt_score_constants::kInfinity, 0, move);
    return move;
}
