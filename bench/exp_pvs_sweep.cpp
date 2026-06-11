//
// Created by Marvin Becker on 11.06.26.
//

/**
 * @file exp_pvs_sweep.cpp
 * @brief Experiment 4: grid over the two PVS parameters at a fixed budget.
 *
 * Sweeps PvsMinDepth x PvsScoutAfterMove and records cost (nodes, depth,
 * re-searches) per combination. The quality side comes later from the
 * Stockfish scoring of the recorded best moves.
 */

#include "bench_common.h"
#include "bench_experiments.h"

namespace bench
{

void run_pvs_sweep(const std::vector<std::string>& args)
{
    const std::string SUITE = get_arg(args, "--suite", "tests/data/thesis-positions.epd");
    const std::string OUT = get_arg(args, "--out", "results");
    const int BUDGET = get_int_arg(args, "--budget", 1000);
    const int REPS = get_int_arg(args, "--reps", 3);
    const int MIN_DEPTH_MAX = get_int_arg(args, "--min-depth-max", 6);
    const int SCOUT_AFTER_MAX = get_int_arg(args, "--scout-after-max", 5);

    const auto FENS = load_suite(SUITE);

    std::ostringstream config;
    config << "suite=" << SUITE << " budget_ms=" << BUDGET << " reps=" << REPS << " min_depth=1.."
           << MIN_DEPTH_MAX << " scout_after=1.." << SCOUT_AFTER_MAX
           << " positions=" << FENS.size();

    auto csv = open_csv(OUT, "pvs_sweep", config.str());
    csv << "fen_id,fen,min_depth,scout_after,rep,completed_depth,seldepth,nodes,qnodes,"
        << "researches,time_ms,bestmove\n";

    ChessBot bot;
    Board board;

    for (int min_depth = 1; min_depth <= MIN_DEPTH_MAX; min_depth++)
    {
        for (int scout_after = 1; scout_after <= SCOUT_AFTER_MAX; scout_after++)
        {
            bot.set_pvs_min_depth(min_depth);
            bot.set_pvs_scout_after_move(scout_after);

            for (std::size_t fen_id = 0; fen_id < FENS.size(); fen_id++)
            {
                board.read_fen(FENS[fen_id]);

                for (int rep = 0; rep < REPS; rep++)
                {
                    // Fresh TT per run, same reasoning as in depth-vs-time.
                    bot.reset_tt();

                    SearchConstraints limits;
                    limits.mode_ = SearchType::FixedTime;
                    limits.movetime_ms_ = BUDGET;

                    const SearchSample SAMPLE = run_search(bot, board, limits);

                    csv << fen_id << "," << FENS[fen_id] << "," << min_depth << "," << scout_after
                        << "," << rep << "," << SAMPLE.completed_depth << "," << SAMPLE.seldepth
                        << "," << SAMPLE.nodes << "," << SAMPLE.qnodes << "," << SAMPLE.researches
                        << "," << SAMPLE.time_ms << "," << SAMPLE.move.to_string() << "\n";
                }
            }

            std::cout << "Config min_depth=" << min_depth << " scout_after=" << scout_after
                      << " done" << std::endl;
        }
    }
}

} // namespace bench
