//
// Created by Marvin Becker on 11.06.26.
//

/**
 * @file exp_time_to_quality.cpp
 * @brief Experiment 2: which move does the engine pick per time budget?
 *
 * This experiment only records the chosen moves, the quality side (centipawn
 * loss against Stockfish) is added afterwards by scripts/score_with_stockfish.py.
 * Keeping measurement and scoring separate means we can re-score old runs
 * without re-measuring anything.
 */

#include "bench_common.h"
#include "bench_experiments.h"

namespace bench
{

void run_time_to_quality(const std::vector<std::string>& args)
{
    const std::string SUITE = get_arg(args, "--suite", "tests/data/thesis-positions.epd");
    const std::string OUT = get_arg(args, "--out", "results");
    const int REPS = get_int_arg(args, "--reps", 3);
    const auto BUDGETS =
        parse_int_list(get_arg(args, "--budgets", "10,25,50,100,250,500,1000,2000,5000"));

    const auto FENS = load_suite(SUITE);

    std::ostringstream config;
    config << "suite=" << SUITE << " reps=" << REPS << " positions=" << FENS.size();

    auto csv = open_csv(OUT, "time_to_quality", config.str());
    csv << "fen_id,fen,budget_ms,rep,bestmove,completed_depth,nodes,time_ms,stop_reason\n";

    ChessBot bot;
    Board board;

    for (std::size_t fen_id = 0; fen_id < FENS.size(); fen_id++)
    {
        board.read_fen(FENS[fen_id]);

        for (const int BUDGET : BUDGETS)
        {
            for (int rep = 0; rep < REPS; rep++)
            {
                // Fresh TT per run, every budget has to earn its own tree.
                bot.reset_tt();

                SearchConstraints limits;
                limits.mode_ = SearchType::FixedTime;
                limits.movetime_ms_ = BUDGET;

                const SearchSample SAMPLE = run_search(bot, board, limits);

                csv << fen_id << "," << FENS[fen_id] << "," << BUDGET << "," << rep << ","
                    << SAMPLE.move.to_string() << "," << SAMPLE.completed_depth << ","
                    << SAMPLE.nodes << "," << SAMPLE.time_ms << ","
                    << stop_reason_name(SAMPLE.stop_reason) << "\n";
            }
        }

        std::cout << "Position " << (fen_id + 1) << "/" << FENS.size() << " done" << std::endl;
    }
}

} // namespace bench
