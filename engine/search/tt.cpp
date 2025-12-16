//
// Created by Marvin Becker on 16.12.25.
//

#include "tt.h"

bool TranspositionTable::probe(const std::uint64_t KEY, const int DEPTH, const int ALPHA,
                               const int BETA, const int PLY, int& out_score,
                               Move& out_best_move) const
{
    const Entry& e = table_[index(KEY)];

    if (e.generation == 0 || e.key != KEY)
    {
        out_best_move = Move{};
        return false;
    }

    out_best_move = e.best_move;

    // Only use if saved depth is enough
    if (e.depth < DEPTH)
        return false;

    const int score = from_tt_score(e.score, PLY);

    switch (e.flag)
    {
    case TTFlag::Exact:
        out_score = score;
        return true;
    case TTFlag::LowerBound:
        if (score >= BETA)
        {
            out_score = score;
            return true;
        }
        return false;
    case TTFlag::UpperBound:
        if (score <= ALPHA)
        {
            out_score = score;
            return true;
        }
        return false;
    }

    return false;
}

bool TranspositionTable::probe_move(const std::uint64_t KEY, Move& out_best_move) const
{
    const Entry& e = table_[index(KEY)];
    if (e.generation == 0 || e.key != KEY)
        return false;

    out_best_move = e.best_move;
    return true;
}

void TranspositionTable::store(const std::uint64_t KEY, const int DEPTH, const int SCORE,
                               const TTFlag FLAG, const int PLY, const Move& BEST_MOVE)
{
    Entry& e = table_[index(KEY)];

    if (e.key != KEY && e.generation != 0)
    {
        // Replacement Policy modified here for later usage
    }

    const std::uint8_t gen = generation_;
    if (!is_replacement_better(e, DEPTH, gen) && e.key == KEY)
        return;

    e.key = KEY;
    e.depth = DEPTH;
    e.flag = FLAG;
    e.score = to_tt_score(SCORE, PLY);
    e.best_move = BEST_MOVE;
    e.generation = gen;
}
