//
// Created by Marvin Becker on 16.03.24.
//

#include <iostream>
#include <mutex>
#include <sstream>
#include <thread>

#include "chess_game.h"
#include "search/time/search_constraints.h"
#include "search/time/time_manager.h"
#include "search/time/uci_time_control.h"
#include "utils.h"

void ChessGame::start()
{
    parser_init();
}

void ChessGame::stop_search_worker()
{
    std::unique_lock<std::mutex> lk(search_mutex_);

    // Always request stop (harmless if no search is running)
    chess_bot_.request_stop();

    // If a worker thread exists, we MUST join it before reusing search_thread.
    if (search_thread_.joinable())
    {
        lk.unlock();
        search_thread_.join();
        lk.lock();
    }

    search_running_ = false;
}

void ChessGame::clear_ponder_state_locked()
{
    // Caller must hold search_mutex.
    ponder_active_ = false;
    ponder_best_ = Move{};
}

void ChessGame::clear_ponder_state()
{
    std::lock_guard<std::mutex> lk(search_mutex_);
    clear_ponder_state_locked();
}

void ChessGame::parser_uci_handle_position(const std::string& line) const
{
    std::string token;
    std::istringstream iss(line);

    iss >> token; // "position"
    iss >> token; // "startpos" oder "fen"

    if (token == "startpos")
    {
        board_->reset();

        // Check if there are more moves
        if (iss >> token && token == "moves")
        {
            while (iss >> token)
            {
                Move m = board_->parse_move(token);
                board_->try_to_move_piece(m);
            }
        }
    }
    else if (token == "fen")
    {
        std::string fen, part;
        while (iss >> part && part != "moves")
        {
            fen += part + " ";
        }
        board_->read_fen(fen);

        if (part == "moves")
        {
            while (iss >> token)
            {
                Move m = board_->parse_move(token);
                board_->try_to_move_piece(m);
            }
        }
    }
}

void ChessGame::parser_uci_handle_go(const std::string& line)
{
    // Parse the UCI "go" command into search constraints + raw time control.
    // See: https://backscattering.de/chess/uci/

    SearchConstraints constraints{};
    UciTimeControl utc{};

    // Defaults: no explicit constraints.
    constraints.mode_ = SearchType::Normal;
    constraints.movetime_ms_ = -1;
    constraints.depth_ = -1;
    constraints.nodes_ = -1;

    // Track whether any time-related token was provided.
    bool saw_any_time_token = false;

    std::istringstream iss(line);
    std::string token;
    iss >> token; // "go"

    while (iss >> token)
    {
        if (token == "infinite")
            constraints.mode_ = SearchType::Infinite;

        else if (token == "ponder")
            constraints.mode_ = SearchType::Ponder;

        else if (token == "depth")
        {
            iss >> constraints.depth_;
            constraints.mode_ = SearchType::FixedDepth;
        }
        else if (token == "nodes")
        {
            iss >> constraints.nodes_;
            constraints.mode_ = SearchType::NodeLimit;
        }
        else if (token == "movetime")
        {
            iss >> constraints.movetime_ms_;
            constraints.mode_ = (constraints.mode_ == SearchType::Ponder) ? SearchType::Ponder
                                                                          : SearchType::FixedTime;
        }
        else if (token == "wtime")
        {
            iss >> utc.wtime_;
            saw_any_time_token = true;
        }
        else if (token == "btime")
        {
            iss >> utc.btime_;
            saw_any_time_token = true;
        }
        else if (token == "winc")
        {
            iss >> utc.winc_;
            saw_any_time_token = true;
        }
        else if (token == "binc")
        {
            iss >> utc.binc_;
            saw_any_time_token = true;
        }
        else if (token == "movestogo")
        {
            iss >> utc.movestogo_;
            saw_any_time_token = true;
        }
    }

    // If a tournament clock was provided, compute a per-move budget.
    // (FixedTime/FixedDepth/NodeLimit/Infinite/Ponder do not require tc.)
    if (constraints.mode_ == SearchType::Normal && saw_any_time_token)
        constraints.budget_ = search::time::TimeManager::compute_budget(board_->player_, utc);

    // Stop previous search if exists.
    stop_search_worker();

    // Start search and set pondering if needed.
    {
        std::lock_guard<std::mutex> lk(search_mutex_);
        search_running_ = true;

        ponder_active_ = (constraints.mode_ == SearchType::Ponder);
        ponder_best_ = Move{};
    }

    Board board_copy = *board_;
    SearchConstraints constraints_copy = constraints;

    search_thread_ = std::thread([this, board_copy, constraints_copy]() mutable {
        const Move best = chess_bot_.think(board_copy, constraints_copy);

        // If Ponder, dont print and save the best move.
        if (constraints_copy.mode_ == SearchType::Ponder)
        {
            std::lock_guard<std::mutex> lk(search_mutex_);
            ponder_best_ = best;
            search_running_ = false;
            return;
        }

        std::cout << "bestmove " << best.to_string() << std::endl;

        std::lock_guard<std::mutex> lk(search_mutex_);
        search_running_ = false;
    });
}

void ChessGame::parser_parse_uci(const std::string& line)
{
    if (line.empty())
        return;

    if (line == "uci")
    {
        std::cout << "id name Helix" << std::endl;
        std::cout << "id author Marvin Becker" << std::endl;
        std::cout << "option name PvsMinDepth type spin default 2 min 1 max 64" << std::endl;
        std::cout << "option name PvsScoutAfterMove type spin default 1 min 1 max 64" << std::endl;
        std::cout
            << "option name Debug type combo default none var none var basic var medium var verbose"
            << std::endl;
        std::cout << "uciok" << std::endl;
        return;
    }
    if (line == "isready")
    {
        std::cout << "readyok" << std::endl;
        return;
    }
    if (line == "stop")
    {
        stop_search_worker();
        clear_ponder_state();
        return;
    }
    if (line == "ponderhit")
    {
        // Only meaningful if a ponder search is active.
        {
            std::lock_guard<std::mutex> lk(search_mutex_);
            if (!ponder_active_)
                return;
        }

        stop_search_worker();

        Move best;
        {
            std::lock_guard<std::mutex> lk(search_mutex_);
            best = ponder_best_;
            clear_ponder_state_locked();
        }

        // Could be null if ponderhit is too fast.
        if (best.is_null())
            best = moveGenUtils::get_legal_fallback_move(*board_);

        std::cout << "bestmove " << best.to_string() << std::endl;
        return;
    }
    if (line == "ucinewgame")
    {
        stop_search_worker();
        clear_ponder_state();
        board_->reset();
        chess_bot_.reset_tt();
        return;
    }

    if (starts_with(line, "position "))
    {
        stop_search_worker();
        clear_ponder_state();
        parser_uci_handle_position(line);
        return;
    }

    if (starts_with(line, "go"))
    {
        parser_uci_handle_go(line);
        return;
    }

    if (starts_with(line, "setoption "))
    {
        std::istringstream iss(line);
        std::string token, name, value;

        iss >> token; // setoption
        iss >> token; // name
        iss >> name;  // Debug

        if (name == "PvsMinDepth")
        {
            iss >> token; // value
            iss >> value;

            try
            {
                chess_bot_.set_pvs_min_depth(std::stoi(value));
            }
            catch (const std::exception&)
            {
                // Ignore malformed values.
            }

            return;
        }

        if (name == "PvsScoutAfterMove")
        {
            iss >> token; // value
            iss >> value;

            try
            {
                chess_bot_.set_pvs_scout_after_move(std::stoi(value));
            }
            catch (const std::exception&)
            {
                // Ignore malformed values.
            }

            return;
        }

        if (name == "Debug")
        {
            iss >> token; // value
            iss >> value;

            if (value == "none")
            {
                chess_bot_.set_debug_enabled(false);
                return;
            }

            if (value == "basic")
                chess_bot_.set_debug_level(ChessBot::DebugLevel::BASIC);

            else if (value == "medium")
                chess_bot_.set_debug_level(ChessBot::DebugLevel::MEDIUM);

            else if (value == "verbose")
                chess_bot_.set_debug_level(ChessBot::DebugLevel::VERBOSE);

            chess_bot_.set_debug_enabled(true);
        }

        return;
    }
    if (line == "quit")
    {
        stop_search_worker();
        exit(0);
    }

    // Unknown commands are ignored.
}

void ChessGame::parser_parse_classic(const std::string& line)
{
    if (line.empty())
        return;

    if (line[0] == 'F')
    {
        // Read in FEN notation.
        board_->read_fen(line.substr(1, line.length()));
        board_->print_current_board();
        return;
    }
    if (line[0] == 'f')
    {
        // Get the FEN.
        std::cout << "Your FEN: " << board_->get_fen() << std::endl;
        return;
    }

    // undo the last two moves. (Bot did also move that's why)
    if (line == "undo")
    {
        board_->pop_last_move();
        board_->pop_last_move();
        board_->print_current_board();
        return;
    }

    // Check if input has the correct length.
    if (line.length() < 4)
    {
        std::cout << "invalid" << std::endl;
        return;
    }

    // Parse the move.

    if (const Move PLAYER_MOVE = board_->parse_move(line); board_->try_to_move_piece(PLAYER_MOVE))
    {
        // Make the move.
        if (board_->is_check_mate(board_->player_ == WHITE))
        {
            // Check the opponent for check mate.
            std::cout << "CHECK MATE!" << std::endl;
            return;
        }

        constexpr auto LIMIT =
            SearchConstraints{SearchType::Normal, -1, -1, -1, {1800, 2000, 2000}};

        // Bot can only move legal so no need to check if the move is legal.
        // Check if opponent is in check mate after bots turn.
        const Move MOVE = chess_bot_.think(*board_, LIMIT);
        board_->make_move(MOVE);
        board_->print_current_board();

        if (board_->is_check_mate(board_->player_ == WHITE))
        {
            // Check the opponent for check mate.
            board_->print_current_board();
            std::cout << "CHECK MATE!" << std::endl;
        }
    }
    else
        std::cout << "invalid" << std::endl;
}

void ChessGame::parser_init()
{
    bool uci_mode = false;

    // Loop through the input.
    std::string input;
    while (getline(std::cin, input))
    {
        // If uci parser does accept string, continue
        if (input == "uci")
            uci_mode = true;
        else if (input == "classic")
        {
            uci_mode = false;
            board_->print_current_board();
            continue;
        }

        // If uci mode is activated
        if (uci_mode)
        {
            parser_parse_uci(input);
            continue;
        }

        parser_parse_classic(input);
    }
}
