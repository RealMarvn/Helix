//
// Created by Marvin Becker on 10.02.24.
//
#include "search.h"

#include "./eval/eval.h"
#include "time/time_manager.h"

#include <csignal>
#include <iostream>

void ChessBot::reset_tt()
{
    tt.clear();
}

bool ChessBot::hard_stop() const
{
    // Hard time limit
    if (constraint.budget.hard_time_up(search::time::TimeManager::now_ms()))
        return true;

    // Node limit
    if (constraint.has_node_limit() && nodes_searched >= constraint.nodes)
        return true;

    return false;
}

void ChessBot::reset_search_state()
{
    nodes_searched = 0; // reset the nodes for new search
    tt.new_search();    // reset transposition table
    killers.clear();    // reset killer table
    history.clear();    // reset history table
}

Move ChessBot::think(Board board, SearchConstraints config)
{
    // save constraint as member of ChessBot
    this->constraint = config;

    // Reset search state
    reset_search_state();

    switch (config.mode)
    {
    case SearchType::FixedDepth: {
        // Reset time limit so hard_stop does not kill the search.
        this->constraint.budget = {};

        Move move = pick_fallback_root_move(board);
        root_search(board, config.depth, move);
        return move;
    }
    case SearchType::NodeLimit:
    default: {
        // Initialize timers for time based search
        search::time::TimeManager::init_search(constraint);
        return iterative_deepening(board);
    }
    }
}

Move ChessBot::pick_fallback_root_move(Board& board)
{
    auto moves = moveGenUtils::get_all_pseudo_legal_moves(board, board.player == WHITE);

    for (Move& m : moves)
    {
        if (board.make_move(m))
        {
            board.pop_last_move();
            return m;
        }
    }

    // No legal moves (checkmate or stalemate)
    return Move{};
}

Move ChessBot::iterative_deepening(Board& board)
{
    Move bestMove = pick_fallback_root_move(board);
    for (int i = 1;; i++)
    {
        // New move for each iteration.
        Move move{};

        // Search the best move with depth i.
        // root_search returns true if it got aborted (don't trust result).
        if (root_search(board, i, move))
            break;

        // Set the move if the search is fully done.
        bestMove = move;

        // Check soft budget.
        if (constraint.budget.soft_time_up(search::time::TimeManager::now_ms()))
            break;
    }
    return bestMove;
}

ChessBot::SearchResult ChessBot::negamax(Board& board, const int DEPTH, int alpha, const int BETA,
                                         const int PLY, Move& best_move)
{
    // Count nodes.
    ++nodes_searched;

    // Leaf node? → Quiescence.
    if (DEPTH <= 0)
        return quiescence(board, alpha, BETA);

    // Time limit check.
    if (hard_stop())
        return {-tt_score_constants::kInfinity, true};

    // Remember original alpha value because of tt.
    const int ORIGINAL_ALPHA = alpha;
    const std::uint64_t key = board.get_hash();

    // Transposition table probe (may return exact score or safe cutoff)
    Move tt_move{};
    if (int tt_score = 0; tt.probe(key, DEPTH, alpha, BETA, PLY, tt_score, tt_move))
    {
        return {tt_score, false};
    }

    // Get all possible moves.
    auto moveList = moveGenUtils::get_all_pseudo_legal_moves(board, board.player == WHITE);

    // Sort so the best moves are first (TT move + captures + killer/history heuristics).
    search::heuristics::order_moves(moveList, tt_move, PLY, board.player, killers, history);

    int legalMoves = 0;

    // First best score should be initialized with worst value.
    int bestScore = -tt_score_constants::kInfinity;
    Move localBestMove{};

    for (Move& move : moveList)
    {
        int score;

        // Make every move and gather the value of the opponent.
        if (board.make_move(move))
        {
            Move child_best{};
            auto [result_score, aborted] =
                negamax(board, DEPTH - 1, -BETA, -alpha, PLY + 1, child_best);

            // Pop the last move to clean the board.
            board.pop_last_move();

            // Abort too.
            if (aborted)
                return {-tt_score_constants::kInfinity, true};

            score = -result_score;
            legalMoves++;
        }
        else
        {
            continue;
        }

        // Set the best score if the current one is less.
        if (score > bestScore)
        {
            bestScore = score;
            localBestMove = move;

            // Only if we are PLY == 0.
            if (PLY == 0)
                best_move = move;
        }

        // Start pruning with alpha and beta.
        if (bestScore > alpha)
            alpha = bestScore;

        if (alpha >= BETA)
        {
            // Beta-cutoff: update quiet-move heuristics (killer + history)
            if (search::heuristics::is_quiet(move))
            {
                killers.add(PLY, move);
                history.add(board.player, move.square, move.move_square, DEPTH);
            }

            break; // beta kill
        }
    }

    // If no legal moves left.
    if (legalMoves == 0)
    {
        if (board.is_king_in_check(board.player == WHITE))
        {
            return {-tt_score_constants::kMate + PLY, false}; // checkmate
        }
        else
        {
            return {0, false}; // stalemate
        }
    }

    // Store to transposition table
    auto flag = TTFlag::Exact;
    if (bestScore <= ORIGINAL_ALPHA)
        flag = TTFlag::UpperBound;
    else if (bestScore >= BETA)
        flag = TTFlag::LowerBound;

    tt.store(key, DEPTH, bestScore, flag, PLY, localBestMove);

    return {bestScore, false};
}

ChessBot::SearchResult ChessBot::quiescence(Board& board, int alpha, const int BETA)
{
    // Count nodes.
    ++nodes_searched;

    // Hard stop if reached.
    if (hard_stop())
        return {-tt_score_constants::kInfinity, true};

    // Eval the position.
    const int STAND_PAT = eval::evaluate(board);

    if (STAND_PAT >= BETA)
    {
        return {BETA, false};
    }
    if (alpha < STAND_PAT)
    {
        alpha = STAND_PAT;
    }

    // Get all possible moves.
    Move tt_move{};
    tt.probe_move(board.get_hash(), tt_move);
    auto moveList = moveGenUtils::get_all_pseudo_legal_moves(board, board.player == WHITE);

    // Sort so the best moves are first (TT move + captures + killer/history heuristics).
    search::heuristics::order_moves(moveList, tt_move, 0, board.player, killers, history);

    // The best score should be initialized with the worst value possible.
    int bestScore = STAND_PAT;

    for (Move& move : moveList)
    {
        int score;

        // Skip the normal moves so I only have captures.
        if (move.captured_piece.piece_type == EMPTY && move.move_type != EN_PASSANT)
            continue;

        // Make every move and gather the value of the opponent.
        if (board.make_move(move))
        {
            const auto [result_score, aborted] = quiescence(board, -BETA, -alpha);

            // Pop last move made.
            board.pop_last_move();

            // If aborted, abort too.
            if (aborted)
                return {-tt_score_constants::kInfinity, true};

            score = -result_score;
        }
        else
            continue;

        // Update best score.
        bestScore = std::max(bestScore, score);

        // if score is less than alpha continue.
        if (score <= alpha)
            continue;

        // Set the score.
        alpha = score;
        if (score >= BETA)
            break;
    }

    return {bestScore, false};
}

bool ChessBot::root_search(Board& board, const int DEPTH, Move& move)
{
    const auto [score, aborted] = negamax(board, DEPTH, -tt_score_constants::kInfinity,
                                          tt_score_constants::kInfinity, 0, move);

    return aborted;
}
