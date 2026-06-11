//
// Created by Marvin Becker on 11.06.26.
//

/**
 * @file exp_depth_vs_time.cpp
 * @brief Experiment 3: reachable depth per time budget, with and without PVS.
 *
 * Every position is searched once per budget and repetition with a fresh TT,
 * so the budgets do not profit from each other. PVS is switched off by
 * raising the scout threshold so high that the scout never kicks in, the
 * search code itself stays untouched.
 */

#include "bench_common.h"
#include "bench_experiments.h"

namespace bench
{

void run_depth_vs_time(const std::vector<std::string>& args)
{
    const std::string SUITE = get_arg(args, "--suite", "tests/data/thesis-positions.epd");
    const std::string OUT = get_arg(args, "--out", "results");
    const std::string PVS = get_arg(args, "--pvs", "on");
    const int REPS = get_int_arg(args, "--reps", 3);
    const auto BUDGETS = parse_int_list(get_arg(args, "--budgets", "10,25,50,100,250,500,1000"));

    const auto FENS = load_suite(SUITE);
    const bool PVS_ON = (PVS == "on");

    std::ostringstream config;
    config << "suite=" << SUITE << " pvs=" << PVS << " reps=" << REPS
           << " positions=" << FENS.size();

    auto csv = open_csv(OUT, "depth_vs_time", config.str());
    csv << "fen_id,budget_ms,rep,pvs,completed_depth,seldepth,nodes,qnodes,researches,"
        << "time_ms,bestmove,stop_reason\n";

    ChessBot bot;
    Board board;

    // PVS off = the scout never triggers because no search gets that deep.
    if (!PVS_ON)
        bot.set_pvs_min_depth(64);

    for (std::size_t fen_id = 0; fen_id < FENS.size(); fen_id++)
    {
        board.read_fen(FENS[fen_id]);

        for (const int BUDGET : BUDGETS)
        {
            for (int rep = 0; rep < REPS; rep++)
            {
                // Fresh TT per run, otherwise we would measure tree reuse here.
                bot.reset_tt();

                SearchConstraints limits;
                limits.mode_ = SearchType::FixedTime;
                limits.movetime_ms_ = BUDGET;

                const SearchSample SAMPLE = run_search(bot, board, limits);

                csv << fen_id << "," << BUDGET << "," << rep << "," << (PVS_ON ? "on" : "off")
                    << "," << SAMPLE.completed_depth << "," << SAMPLE.seldepth << ","
                    << SAMPLE.nodes << "," << SAMPLE.qnodes << "," << SAMPLE.researches << ","
                    << SAMPLE.time_ms << "," << SAMPLE.move.to_string() << ","
                    << stop_reason_name(SAMPLE.stop_reason) << "\n";
            }
        }

        std::cout << "Position " << (fen_id + 1) << "/" << FENS.size() << " done" << std::endl;
    }
}

} // namespace bench
