//
// Created by Marvin Becker on 11.06.26.
//

/**
 * @file bench_main.cpp
 * @brief Entry point of the bench harness (helix-bench).
 *
 * Dispatches to the single experiments. Run without arguments to get the
 * usage overview. The harness links the engine as a library, the engine
 * code itself is not modified by any measurement.
 */

#include <iostream>
#include <string>
#include <vector>

#include "bench_experiments.h"

static void print_usage()
{
    std::cout
        << "Usage: helix-bench <experiment> [options]\n"
        << "\n"
        << "Experiments:\n"
        << "  move-stability   --suite <epd> [--out results] [--max-depth 10]\n"
        << "  depth-vs-time    --suite <epd> [--out results] [--budgets "
           "10,25,50,100,250,500,1000]\n"
        << "                   [--reps 3] [--pvs on|off]\n"
        << "  pvs-sweep        --suite <epd> [--out results] [--budget 1000] [--reps 3]\n"
        << "                   [--min-depth-max 6] [--scout-after-max 5]\n"
        << "  time-to-quality  --suite <epd> [--out results] [--budgets ...] [--reps 3]\n"
        << "  tree-reuse       --games <txt> [--out results] [--budget 500] [--mode keep|clear]\n"
        << "\n"
        << "Suite format:  one FEN per line, everything after ';' is ignored (EPD).\n"
        << "Games format:  one game per line as UCI moves from the start position,\n"
        << "               e.g. \"e2e4 e7e5 g1f3 ...\" (see scripts/pgn_to_uci.py).\n";
}

int main(const int argc, const char** argv)
{
    if (argc < 2)
    {
        print_usage();
        return 1;
    }

    const std::string EXPERIMENT = argv[1];

    // Everything after the experiment name goes to the experiment itself.
    const std::vector<std::string> args(argv + 2, argv + argc);

    if (EXPERIMENT == "move-stability")
        bench::run_move_stability(args);
    else if (EXPERIMENT == "depth-vs-time")
        bench::run_depth_vs_time(args);
    else if (EXPERIMENT == "pvs-sweep")
        bench::run_pvs_sweep(args);
    else if (EXPERIMENT == "time-to-quality")
        bench::run_time_to_quality(args);
    else if (EXPERIMENT == "tree-reuse")
        bench::run_tree_reuse(args);
    else
    {
        std::cerr << "Unknown experiment: " << EXPERIMENT << "\n\n";
        print_usage();
        return 1;
    }

    return 0;
}
