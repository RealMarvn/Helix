//
// Created by Marvin Becker on 16.12.25.
//

#include "tt.h"

bool TranspositionTable::probe(const std::uint64_t key, const int depth, const int alpha,
                               const int beta, const int ply, int& out_score,
                               Move& out_best_move) const
{
    // Update stats.
    ++stats_.probes_;

    // Get entries with key.
    const auto& [TT_KEY, TT_SCORE, TT_DEPTH, TT_FLAG, TT_GENERATION, TT_BEST_MOVE] =
        table_[index(key)];

    if (TT_GENERATION == 0 || TT_KEY != key)
    {
        out_best_move = Move{};
        return false;
    }

    ++stats_.hits_;
    out_best_move = TT_BEST_MOVE;

    // Only use if saved depth is enough.
    if (TT_DEPTH < depth)
        return false;

    const int score = from_tt_score(TT_SCORE, ply);

    switch (TT_FLAG)
    {
    case TTFlag::EXACT:
        out_score = score;
        return true;

    case TTFlag::LOWER_BOUND:
        if (score >= beta)
        {
            out_score = score;
            return true;
        }
        return false;

    case TTFlag::UPPER_BOUND:
        if (score <= alpha)
        {
            out_score = score;
            return true;
        }
        return false;
    }

    return false;
}

bool TranspositionTable::probe_move(const std::uint64_t key, Move& out_best_move) const
{
    ++stats_.probes_;

    const Entry& e = table_[index(key)];
    if (e.generation_ == 0 || e.key_ != key)
        return false;

    ++stats_.hits_;
    out_best_move = e.best_move_;
    return true;
}

void TranspositionTable::store(const std::uint64_t key, const int depth, const int score,
                               const TTFlag flag, const int ply, const Move& best_move)
{
    Entry& e = table_[index(key)];

    if (e.key_ != key && e.generation_ != 0)
    {
        // Replacement Policy modify for later usage in the future.
    }

    // For later stats updating.
    const bool WILL_REPLACE_OTHER = (e.generation_ != 0 && e.key_ != key);

    const std::uint8_t gen = generation_;
    if (!is_replacement_better(e, depth, gen) && e.key_ == key)
        return;

    // Update stats.
    ++stats_.stores_;
    if (WILL_REPLACE_OTHER)
        ++stats_.replaces_;

    e.key_ = key;
    e.depth_ = depth;
    e.flag_ = flag;
    e.score_ = to_tt_score(score, ply);
    e.best_move_ = best_move;
    e.generation_ = gen;
}
