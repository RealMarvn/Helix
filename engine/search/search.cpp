//
// Created by Marvin Becker on 10.02.24.
//
#include "search.h"

#include "./eval/eval.h"
#include "time/time_manager.h"

#include <algorithm>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_set>

void ChessBot::reset_tt()
{
    tt.clear();
}

bool ChessBot::hard_stop()
{
    // Stop flag check.
    if (stop_requested.load(std::memory_order_relaxed))
    {
        stop_reason = STOP_FLAG;
        return true;
    }

    // Hard time limit.
    if (constraint.budget_.hard_time_up(search::time::TimeManager::now_ms()))
    {
        stop_reason = HARD_TIME;
        return true;
    }

    // Node limit.
    if (constraint.has_node_limit() && nodes >= constraint.nodes_)
    {
        stop_reason = NODE_LIMIT;
        return true;
    }

    return false;
}

void ChessBot::reset_search_state()
{
    clear_stop();       // Resets the stop state.
    nodes = 0;          // Reset the nodes for new search.
    qnodes = 0;         // Reset the Qnodes for new search
    seldepth = 0;       // Resets the seldepth for a new search.
    stop_reason = NONE; // Resets the stop reason for a new search.
    tt_returns = 0;     // Resets the tt_returns counter for a new search.
    tt.new_search();    // Reset transposition table stats and age it.
    killers.clear();    // Reset killer table.
    history.clear();    // Reset history table.
}

void ChessBot::print_info(const int depth, const int score, const Move& pv_move,
                          const long long start_time_ms) const
{
    const auto NOW = search::time::TimeManager::now_ms();
    const auto TIME_MS = NOW - start_time_ms;

    const long long NPS = TIME_MS > 0 ? (nodes * 1000) / TIME_MS : 0;

    std::cout << "info"
              << " depth " << depth << " seldepth " << seldepth << " time " << TIME_MS << " nodes "
              << nodes << " nps " << NPS;

    if (constexpr int MATE_THRESHOLD = tt_score_constants::kMate - 1000;
        std::abs(score) >= MATE_THRESHOLD)
    {
        // Convert internal mate score (±(kMate - ply)) to "mate N" where N is in full moves.
        const int PLIES_TO_MATE = tt_score_constants::kMate - std::abs(score);
        int mate_in = (PLIES_TO_MATE + 1) / 2;
        if (score < 0)
            mate_in = -mate_in;

        std::cout << " score mate " << mate_in;
    }
    else
        std::cout << " score cp " << score;

    std::cout << " pv " << pv_move.to_string() << std::endl;
}

void ChessBot::print_debug(Board& board, const int depth, const int score,
                           const long long start_time_ms) const
{
    if (!debug.enabled)
        return;

    const long long now_ms = search::time::TimeManager::now_ms();
    const long long time_ms = now_ms - start_time_ms;
    const long long nps = (time_ms > 0) ? (nodes * 1000) / time_ms : 0;

    std::cout << "info string DBG"
              << " depth=" << depth << " seldepth=" << seldepth << " nodes=" << nodes
              << " time=" << time_ms << " nps=" << nps
              << " reason=" << stop_reason_to_cstr(stop_reason) << " score=" << score << std::endl;

    if (debug.level >= DebugLevel::MEDIUM)
    {
        search::debug::print_health(*this);
        search::debug::print_tt(*this);
    }

    if (debug.level >= DebugLevel::VERBOSE)
    {
        search::debug::print_root_ordering(*this, board);
        search::debug::print_pv(*this, board);
    }
}

Move ChessBot::think(Board board, SearchConstraints config /* intentional copy */)
{
    // save constraint as member of ChessBot.
    this->constraint = config;

    // Reset search state.
    reset_search_state();

    switch (config.mode_)
    {
    case SearchType::FixedDepth: {
        // Reset the time limit so hard_stop does not kill the search.
        this->constraint.budget_ = {};
        const long long START_TIME_MS = search::time::TimeManager::now_ms();

        Move move = moveGenUtils::get_legal_fallback_move(board);
        const auto [SCORE, ABORTED] = root_search(board, config.depth_, move);

        if (!ABORTED)
            print_info(config.depth_, SCORE, move, START_TIME_MS);

        print_debug(board, config.depth_, SCORE, START_TIME_MS);

        // Check legality before returning!
        if (!board.is_legal_by_make_unmake(move))
            move = moveGenUtils::get_legal_fallback_move(board);

        return move;
    }
    case SearchType::NodeLimit:
    case SearchType::Infinite: {
        // Reset the time limit so hard_stop does not kill the search.
        this->constraint.budget_ = {};
        return iterative_deepening(board);
    }
    default: {
        // Initialize timers for time-based search.
        search::time::TimeManager::init_search(constraint);
        return iterative_deepening(board);
    }
    }
}

Move ChessBot::iterative_deepening(Board& board)
{
    Move bestMove = moveGenUtils::get_legal_fallback_move(board);
    const long long START_TIME_MS = search::time::TimeManager::now_ms();

    for (int i = 1;; i++)
    {
        // New move for each iteration.
        Move move = bestMove;

        // Search the best move with depth i.
        auto [score, aborted] = root_search(board, i, move);

        print_debug(board, i, score, START_TIME_MS);

        // root_search returns true if it got aborted (don't trust result).
        if (aborted)
            break;

        // Set the move if the search is fully done.
        bestMove = move;

        print_info(i, score, move, START_TIME_MS);

        // Check soft budget.
        if (constraint.budget_.soft_time_up(search::time::TimeManager::now_ms()))
        {
            stop_reason = SOFT_TIME;
            break;
        }
    }

    if (!board.is_legal_by_make_unmake(bestMove))
        bestMove = moveGenUtils::get_legal_fallback_move(board);

    return bestMove;
}

ChessBot::SearchResult ChessBot::negamax(Board& board, const int depth, int alpha, const int beta,
                                         const int ply, Move& best_move)
{
    // Update stats.
    updateStats(ply);

    // Leaf node? → Quiescence.
    if (depth <= 0)
        return quiescence(board, alpha, beta, ply);

    // Time limit check.
    if (hard_stop())
        return {-tt_score_constants::kInfinity, true};

    // Remember original alpha value because of tt.
    const int ORIGINAL_ALPHA = alpha;
    const std::uint64_t key = board.get_hash();

    // Transposition table probe (may return exact score or safe cutoff).
    Move tt_move{};
    if (int tt_score = 0; tt.probe(key, depth, alpha, beta, ply, tt_score, tt_move))
    {
        // Check if we are in the root, the move is not null and legal!
        if (ply == 0 && !tt_move.is_null() && board.is_legal_by_make_unmake(tt_move))
            best_move = tt_move;

        ++tt_returns;
        return {tt_score, false};
    }

    // If tt is not legal, reset it!
    if (!tt_move.is_null() && !board.is_legal_by_make_unmake(tt_move))
        tt_move = Move{};

    // Get all possible moves.
    auto moveList = moveGenUtils::get_pseudo_legal_moves(board, board.player_ == WHITE);

    // Sort so the best moves are first (TT move + captures + killer/history heuristics).
    search::heuristics::order_moves(moveList, tt_move, ply, board.player_, killers, history);

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
            int result_score;
            bool aborted;

            // Search full window for the first scout_after_move moves.
            // Or when the depth is too shallow.
            if (legalMoves < pvs.scout_after_move || depth < pvs.min_depth)
            {
                // Search with full window.
                auto r = negamax(board, depth - 1, -beta, -alpha, ply + 1, child_best);
                result_score = r.score;
                aborted = r.aborted;
            }
            else
            {
                // Search with Null-Window to check if alpha can be beaten.
                auto r = negamax(board, depth - 1, -alpha - 1, -alpha, ply + 1, child_best);
                result_score = r.score;
                aborted = r.aborted;

                // If alpha was beaten, we search with the full window again.
                if (!aborted && -r.score > alpha && -r.score < beta)
                {
                    auto rs = negamax(board, depth - 1, -beta, -alpha, ply + 1, child_best);
                    result_score = rs.score;
                    aborted = rs.aborted;
                }
            }

            // Pop the last move to clean the board.
            board.pop_last_move();

            // Abort too.
            if (aborted)
                return {-tt_score_constants::kInfinity, true};

            score = -result_score;
            legalMoves++;
        }
        else
            continue;

        // Set the best score if the current one is less.
        if (score > bestScore)
        {
            bestScore = score;
            localBestMove = move;

            // Only if we are PLY == 0.
            if (ply == 0)
                best_move = move;
        }

        // Start pruning with alpha and beta.
        if (bestScore > alpha)
            alpha = bestScore;

        if (alpha >= beta)
        {
            // Beta-cutoff: update quiet-move heuristics (killer + history)
            if (search::heuristics::is_quiet(move))
            {
                killers.add(ply, move);
                history.add(board.player_, move.square_, move.move_square_, depth);
            }

            break; // beta kill
        }
    }

    // If no legal moves left.
    if (legalMoves == 0)
    {
        if (board.is_king_in_check(board.player_ == WHITE))
            return {-tt_score_constants::kMate + ply, false}; // checkmate

        return {0, false}; // stalemate
    }

    // Store to transposition table
    auto flag = TTFlag::EXACT;
    if (bestScore <= ORIGINAL_ALPHA)
        flag = TTFlag::UPPER_BOUND;

    else if (bestScore >= beta)
        flag = TTFlag::LOWER_BOUND;

    tt.store(key, depth, bestScore, flag, ply, localBestMove);

    return {bestScore, false};
}

ChessBot::SearchResult ChessBot::quiescence(Board& board, int alpha, const int beta, const int ply)
{
    // update stats
    updateStats(ply);
    ++qnodes;

    // Hard stop if reached.
    if (hard_stop())
        return {-tt_score_constants::kInfinity, true};

    // Eval the position.
    const int STAND_PAT = eval::evaluate(board);

    if (STAND_PAT >= beta)
        return {beta, false};

    if (alpha < STAND_PAT)
        alpha = STAND_PAT;

    // Get all possible moves.
    Move tt_move{};
    tt.probe_move(board.get_hash(), tt_move);
    auto moveList = moveGenUtils::get_pseudo_legal_moves(board, board.player_ == WHITE);

    // Reset tt_move if it is not legal!
    if (!tt_move.is_null() && !board.is_legal_by_make_unmake(tt_move))
        tt_move = Move{};

    // Sort so the best moves are first (TT move + captures + killer/history heuristics).
    search::heuristics::order_moves(moveList, tt_move, ply, board.player_, killers, history);

    // The best score should be initialized with the worst value possible.
    int bestScore = STAND_PAT;

    for (Move& move : moveList)
    {
        int score;

        // Skip the normal moves so I only have captures.
        if (move.captured_piece_.piece_type_ == EMPTY && move.move_type_ != EN_PASSANT)
            continue;

        // Make every move and gather the value of the opponent.
        if (board.make_move(move))
        {
            const auto [result_score, aborted] = quiescence(board, -beta, -alpha, ply + 1);

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
        if (score >= beta)
            break;
    }

    return {bestScore, false};
}

ChessBot::SearchResult ChessBot::root_search(Board& board, const int depth, Move& move)
{
    return negamax(board, depth, -tt_score_constants::kInfinity, tt_score_constants::kInfinity, 0,
                   move);
}
