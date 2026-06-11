//
// Created by Marvin Becker on 11.06.26.
//

/**
 * @file exp_tree_reuse.cpp
 * @brief Experiment 5: what does resuming old search trees actually buy us?
 *
 * Walks through real games move by move and searches every position with a
 * fixed budget. In "keep" mode the TT stays alive over the whole game (the
 * normal engine behavior), in "clear" mode it is wiped before every search.
 * The difference between the two runs is exactly the value of tree reuse.
 *
 * Games file: one game per line as UCI moves from the start position,
 * e.g. "e2e4 e7e5 g1f3 ..." (see scripts/pgn_to_uci.py).
 */

#include "bench_common.h"
#include "bench_experiments.h"

namespace bench
{

/** @brief Read the games file, one vector of UCI move strings per game. */
static std::vector<std::vector<std::string>> load_games(const std::string& path)
{
    std::ifstream file(path);
    if (!file.good())
    {
        std::cerr << "Could not open games file: " << path << std::endl;
        std::exit(1);
    }

    std::vector<std::vector<std::string>> games;
    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '#')
            continue;

        std::vector<std::string> moves;
        std::istringstream ss(line);
        std::string move;
        while (ss >> move)
            moves.push_back(move);

        if (!moves.empty())
            games.push_back(moves);
    }

    if (games.empty())
    {
        std::cerr << "Games file contains no games: " << path << std::endl;
        std::exit(1);
    }

    return games;
}

void run_tree_reuse(const std::vector<std::string>& args)
{
    const std::string GAMES_FILE = get_arg(args, "--games", "tests/data/thesis-games.txt");
    const std::string OUT = get_arg(args, "--out", "results");
    const std::string MODE = get_arg(args, "--mode", "keep");
    const int BUDGET = get_int_arg(args, "--budget", 500);

    const auto GAMES = load_games(GAMES_FILE);
    const bool KEEP_TT = (MODE == "keep");

    std::ostringstream config;
    config << "games=" << GAMES_FILE << " budget_ms=" << BUDGET << " mode=" << MODE
           << " game_count=" << GAMES.size();

    auto csv = open_csv(OUT, "tree_reuse", config.str());
    csv << "game_id,ply,side,mode,completed_depth,seldepth,nodes,qnodes,researches,"
        << "time_ms,bestmove,played_move\n";

    for (std::size_t game_id = 0; game_id < GAMES.size(); game_id++)
    {
        // Fresh engine state per game, like "ucinewgame".
        ChessBot bot;
        Board board;
        board.read_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

        int ply = 0;
        for (const std::string& played : GAMES[game_id])
        {
            // In clear mode we throw the tree away before every search.
            if (!KEEP_TT)
                bot.reset_tt();

            SearchConstraints limits;
            limits.mode_ = SearchType::FixedTime;
            limits.movetime_ms_ = BUDGET;

            const SearchSample SAMPLE = run_search(bot, board, limits);
            const char SIDE = (board.player_ == WHITE) ? 'w' : 'b';

            csv << game_id << "," << ply << "," << SIDE << "," << MODE << ","
                << SAMPLE.completed_depth << "," << SAMPLE.seldepth << "," << SAMPLE.nodes << ","
                << SAMPLE.qnodes << "," << SAMPLE.researches << "," << SAMPLE.time_ms << ","
                << SAMPLE.move.to_string() << "," << played << "\n";

            // Follow the game with the move that was actually played.
            Move move = board.parse_move(played);
            if (!board.make_move(move))
            {
                std::cerr << "Illegal move '" << played << "' in game " << game_id
                          << ", skipping rest of the game." << std::endl;
                break;
            }

            ply++;
        }

        std::cout << "Game " << (game_id + 1) << "/" << GAMES.size() << " done" << std::endl;
    }
}

} // namespace bench
