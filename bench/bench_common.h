//
// Created by Marvin Becker on 11.06.26.
//

/**
 * @file bench_common.h
 * @brief Small helpers shared by all bench experiments.
 *
 * The bench harness is intentionally simple: every experiment loads a
 * position suite, runs searches through the normal ChessBot API and writes
 * one CSV file per run. The engine itself is used exactly like in real play,
 * all stats come out of the SearchReport that think() returns.
 */

#pragma once
#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "../engine/search/search.h"

// Set by meson at configure time, so every CSV knows which engine produced it.
#ifndef BENCH_GIT_HASH
#define BENCH_GIT_HASH "unknown"
#endif

namespace bench
{

/**
 * @brief Everything we want to know about a single search run.
 *
 * Filled by run_search() right after think() returns, so the values always
 * belong to exactly one search.
 */
struct SearchSample
{
    Move move{};
    int completed_depth = 0;
    int seldepth = 0;
    long long nodes = 0;
    long long qnodes = 0;
    long long researches = 0;
    long long time_ms = 0;
    ChessBot::StopReason stop_reason = ChessBot::NONE;
};

/** @brief Run one search, the wall time comes on top of the engine's own report. */
inline SearchSample run_search(ChessBot& bot, const Board& board, const SearchConstraints& LIMITS)
{
    const auto START = std::chrono::steady_clock::now();
    const ChessBot::SearchReport REPORT = bot.think(board, LIMITS);
    const auto END = std::chrono::steady_clock::now();

    SearchSample sample;
    sample.move = REPORT.best_move;
    sample.completed_depth = REPORT.completed_depth;
    sample.seldepth = REPORT.seldepth;
    sample.nodes = REPORT.nodes;
    sample.qnodes = REPORT.qnodes;
    sample.researches = REPORT.researches;
    sample.time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(END - START).count();
    sample.stop_reason = REPORT.stop_reason;
    return sample;
}

/** @brief Human readable stop reason for the CSV. */
inline const char* stop_reason_name(const ChessBot::StopReason R)
{
    return ChessBot::stop_reason_to_cstr(R);
}

/**
 * @brief Load a position suite from an EPD file.
 *
 * Same format as the test data: one position per line, everything after the
 * first ';' is ignored. Empty lines and '#' comments are skipped.
 */
inline std::vector<std::string> load_suite(const std::string& path)
{
    std::ifstream file(path);
    if (!file.good())
    {
        std::cerr << "Could not open suite file: " << path << std::endl;
        std::exit(1);
    }

    std::vector<std::string> fens;
    std::string line;
    while (std::getline(file, line))
    {
        // Cut off EPD opcodes, we only need the FEN part.
        const auto SEMI = line.find(';');
        if (SEMI != std::string::npos)
            line = line.substr(0, SEMI);

        // Trim trailing whitespace so get_fen() comparisons stay clean.
        while (!line.empty() && (line.back() == ' ' || line.back() == '\r'))
            line.pop_back();

        if (line.empty() || line[0] == '#')
            continue;

        fens.push_back(line);
    }

    if (fens.empty())
    {
        std::cerr << "Suite file contains no positions: " << path << std::endl;
        std::exit(1);
    }

    return fens;
}

/** @brief Get the value after an option like "--suite", or the fallback if missing. */
inline std::string get_arg(const std::vector<std::string>& args, const std::string& name,
                           const std::string& fallback)
{
    for (std::size_t i = 0; i + 1 < args.size(); i++)
    {
        if (args[i] == name)
            return args[i + 1];
    }
    return fallback;
}

/** @brief Same as get_arg, but converted to int. */
inline int get_int_arg(const std::vector<std::string>& args, const std::string& name,
                       const int FALLBACK)
{
    const std::string VALUE = get_arg(args, name, "");
    return VALUE.empty() ? FALLBACK : std::stoi(VALUE);
}

/** @brief Parse a comma separated list like "10,25,50" into ints. */
inline std::vector<int> parse_int_list(const std::string& input)
{
    std::vector<int> values;
    std::istringstream ss(input);
    std::string item;
    while (std::getline(ss, item, ','))
        values.push_back(std::stoi(item));
    return values;
}

/** @brief Timestamp like 20260611-153000, used for file names and CSV headers. */
inline std::string timestamp()
{
    const std::time_t NOW = std::time(nullptr);
    std::ostringstream ss;
    ss << std::put_time(std::localtime(&NOW), "%Y%m%d-%H%M%S");
    return ss.str();
}

/**
 * @brief Open the CSV for one run and write the config header.
 *
 * The header lines start with '#' so every result file documents the engine
 * version and the exact settings it was produced with.
 */
inline std::ofstream open_csv(const std::string& out_dir, const std::string& experiment,
                              const std::string& config)
{
    std::filesystem::create_directories(out_dir);
    const std::string PATH = out_dir + "/" + experiment + "_" + timestamp() + ".csv";

    std::ofstream csv(PATH);
    if (!csv.good())
    {
        std::cerr << "Could not create output file: " << PATH << std::endl;
        std::exit(1);
    }

    csv << "# experiment=" << experiment << " engine=helix-" << BENCH_GIT_HASH
        << " date=" << timestamp() << "\n";
    csv << "# " << config << "\n";

    std::cout << "Writing results to " << PATH << std::endl;
    return csv;
}

} // namespace bench
