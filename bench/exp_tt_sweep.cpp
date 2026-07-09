//
// Created by Marvin Becker on 09.07.26.
//

/**
 * @file exp_tt_sweep.cpp
 * @brief Experiment 6: transposition table size at a fixed budget.
 *
 * Sweeps the TT size (in MB) and records depth, nodes, hit rate and
 * replacement pressure per size. Answers two questions at once: how much
 * does the table size matter at this time control, and where does the
 * hit rate saturate.
 */

#include "bench_common.h"
#include "bench_experiments.h"

namespace bench
{

void run_tt_sweep(const std::vector<std::string>& args)
{
    const std::string SUITE = get_arg(args, "--suite", "tests/data/thesis-positions.epd");
    const std::string OUT = get_arg(args, "--out", "results");
    const int BUDGET = get_int_arg(args, "--budget", 1000);
    const int REPS = get_int_arg(args, "--reps", 3);
    const auto SIZES_MB = parse_int_list(get_arg(args, "--mb", "1,4,16,64,256"));

    const auto FENS = load_suite(SUITE);

    std::ostringstream config;
    config << "suite=" << SUITE << " budget_ms=" << BUDGET << " reps=" << REPS << " mb=";
    for (std::size_t i = 0; i < SIZES_MB.size(); i++)
        config << (i ? "," : "") << SIZES_MB[i];
    config << " entry_bytes=" << TranspositionTable::entry_size() << " positions=" << FENS.size();

    auto csv = open_csv(OUT, "tt_sweep", config.str());
    csv << "fen_id,tt_mb,rep,completed_depth,seldepth,nodes,qnodes,"
        << "tt_probes,tt_hits,tt_stores,tt_replaces,tt_hit_rate,"
        << "time_ms,bestmove,stop_reason\n";

    ChessBot bot;
    Board board;

    for (const int MB : SIZES_MB)
    {
        bot.set_tt_size_mb(MB);

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

                const TTStats& TT = SAMPLE.tt_stats;
                const double HIT_RATE =
                    TT.probes_ ? static_cast<double>(TT.hits_) / TT.probes_ : 0.0;

                csv << fen_id << "," << MB << "," << rep << "," << SAMPLE.completed_depth << ","
                    << SAMPLE.seldepth << "," << SAMPLE.nodes << "," << SAMPLE.qnodes << ","
                    << TT.probes_ << "," << TT.hits_ << "," << TT.stores_ << "," << TT.replaces_
                    << "," << std::fixed << std::setprecision(4) << HIT_RATE << ","
                    << SAMPLE.time_ms << "," << SAMPLE.move.to_string() << ","
                    << stop_reason_name(SAMPLE.stop_reason) << "\n";
            }
        }

        std::cout << "TT size " << MB << " MB done" << std::endl;
    }
}

} // namespace bench
