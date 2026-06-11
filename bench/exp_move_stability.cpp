//
// Created by Marvin Becker on 11.06.26.
//

/**
 * @file exp_move_stability.cpp
 * @brief Experiment 1: at which depth does the best move stabilize?
 *
 * For every position we search depth 1, 2, 3, ... with the TT kept warm
 * between the depths (that mimics iterative deepening) and log the best
 * move of every depth. The stabilization depth itself is computed later
 * from the CSV: it is the depth after which the move no longer changes.
 */

#include "bench_common.h"
#include "bench_experiments.h"

namespace bench
{

void run_move_stability(const std::vector<std::string>& args)
{
    const std::string SUITE = get_arg(args, "--suite", "tests/data/thesis-positions.epd");
    const std::string OUT = get_arg(args, "--out", "results");
    const int MAX_DEPTH = get_int_arg(args, "--max-depth", 10);

    const auto FENS = load_suite(SUITE);

    std::ostringstream config;
    config << "suite=" << SUITE << " max_depth=" << MAX_DEPTH << " positions=" << FENS.size();

    auto csv = open_csv(OUT, "move_stability", config.str());
    csv << "fen_id,fen,depth,bestmove,nodes,qnodes,seldepth,time_ms\n";

    ChessBot bot;
    Board board;

    for (std::size_t fen_id = 0; fen_id < FENS.size(); fen_id++)
    {
        board.read_fen(FENS[fen_id]);

        // Fresh TT per position, but warm across the depths of one position.
        bot.reset_tt();

        for (int depth = 1; depth <= MAX_DEPTH; depth++)
        {
            SearchConstraints limits;
            limits.mode_ = SearchType::FixedDepth;
            limits.depth_ = depth;

            const SearchSample SAMPLE = run_search(bot, board, limits);

            csv << fen_id << "," << FENS[fen_id] << "," << depth << "," << SAMPLE.move.to_string()
                << "," << SAMPLE.nodes << "," << SAMPLE.qnodes << "," << SAMPLE.seldepth << ","
                << SAMPLE.time_ms << "\n";
        }

        std::cout << "Position " << (fen_id + 1) << "/" << FENS.size() << " done" << std::endl;
    }
}

} // namespace bench
