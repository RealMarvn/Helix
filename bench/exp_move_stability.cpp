//
// Created by Marvin Becker on 11.06.26.
//

/**
 * @file exp_move_stability.cpp
 * @brief Experiment 1: at which depth does the best move stabilize?
 *
 * Every position is searched once with iterative deepening under a fixed node
 * budget. Iterative deepening already walks depth 1, 2, 3, ... and reports the
 * best move of each completed depth, so a single search gives us the whole
 * sequence for free. The node budget keeps the cost bounded and reproducible
 * (unlike a fixed target depth, which runs away on tactical positions), and it
 * is immune to other load on the machine. The stabilization depth itself is
 * computed later from the CSV: it is the depth after which the move no longer
 * changes.
 */

#include "bench_common.h"
#include "bench_experiments.h"

namespace bench
{

void run_move_stability(const std::vector<std::string>& args)
{
    const std::string SUITE = get_arg(args, "--suite", "tests/data/thesis-positions.epd");
    const std::string OUT = get_arg(args, "--out", "results");
    // Budget per position. Big enough to reach roughly depth 10-12 on normal
    // positions, small enough that tactical monsters can't stall the run.
    const int NODE_BUDGET = get_int_arg(args, "--nodes", 20000000);

    const auto FENS = load_suite(SUITE);

    std::ostringstream config;
    config << "suite=" << SUITE << " node_budget=" << NODE_BUDGET << " positions=" << FENS.size();

    auto csv = open_csv(OUT, "move_stability", config.str());
    csv << "fen_id,fen,depth,bestmove,nodes,qnodes,seldepth,time_ms\n";

    ChessBot bot;
    Board board;

    for (std::size_t fen_id = 0; fen_id < FENS.size(); fen_id++)
    {
        board.read_fen(FENS[fen_id]);

        // Fresh TT per position so one position never leaks into the next.
        bot.reset_tt();

        SearchConstraints limits;
        limits.mode_ = SearchType::NodeLimit;
        limits.nodes_ = NODE_BUDGET;

        // One iterative-deepening search; the per-iteration snapshots hold the
        // best move of every completed depth.
        const ChessBot::SearchReport REPORT = bot.think(board, limits);

        for (const auto& IT : REPORT.iterations)
        {
            csv << fen_id << "," << FENS[fen_id] << "," << IT.depth << "," << IT.move.to_string()
                << "," << IT.nodes << "," << IT.qnodes << "," << IT.seldepth << "," << IT.time_ms
                << "\n";
        }

        std::cout << "Position " << (fen_id + 1) << "/" << FENS.size() << " done" << std::endl;
    }
}

} // namespace bench
