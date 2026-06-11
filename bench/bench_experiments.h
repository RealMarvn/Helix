//
// Created by Marvin Becker on 11.06.26.
//

/**
 * @file bench_experiments.h
 * @brief One entry point per thesis experiment.
 *
 * Every experiment takes the raw command line args (without the experiment
 * name itself), runs its measurements and writes one CSV file.
 */

#pragma once
#include <string>
#include <vector>

namespace bench
{

/** @brief At which depth does the best move stop changing? */
void run_move_stability(const std::vector<std::string>& args);

/** @brief How deep do we get per time budget, with and without PVS? */
void run_depth_vs_time(const std::vector<std::string>& args);

/** @brief Grid over the two PVS parameters at a fixed budget. */
void run_pvs_sweep(const std::vector<std::string>& args);

/** @brief Best move per time budget, scored against Stockfish afterwards. */
void run_time_to_quality(const std::vector<std::string>& args);

/** @brief Searches along real games, with persistent TT vs. cleared TT. */
void run_tree_reuse(const std::vector<std::string>& args);

} // namespace bench
