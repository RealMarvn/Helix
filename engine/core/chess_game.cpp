//
// Created by Marvin Becker on 16.03.24.
//

#include <iostream>
#include <mutex>
#include <sstream>
#include <thread>

#include "../search/time/time_manager.h"
#include "./search/time/search_constraints.h"
#include "./search/time/uci_time_control.h"
#include "chess_game.h"

void ChessGame::start()
{
    parser_init();
}

void ChessGame::stop_search_worker()
{
    std::unique_lock<std::mutex> lk(search_mutex);

    // Always request stop (harmless if no search is running)
    chessBot.request_stop();

    // If a worker thread exists, we MUST join it before reusing search_thread.
    if (search_thread.joinable())
    {
        lk.unlock();
        search_thread.join();
        lk.lock();
    }

    search_running = false;
}

void ChessGame::parser_uci_handle_position(const std::string& LINE) const
{
    std::string token;
    std::istringstream iss(LINE);

    iss >> token; // "position"
    iss >> token; // "startpos" oder "fen"

    if (token == "startpos")
    {
        board->reset();

        // Check if there are more moves
        if (iss >> token && token == "moves")
        {
            while (iss >> token)
            {
                Move m = board->parse_move(token);
                board->try_to_move_piece(m);
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
        board->read_fen(fen);

        if (part == "moves")
        {
            while (iss >> token)
            {
                Move m = board->parse_move(token);
                board->try_to_move_piece(m);
            }
        }
    }
}

void ChessGame::parser_uci_handle_go(const std::string& LINE)
{
    // Parse the UCI "go" command into search constraints + raw time control.
    // See: https://backscattering.de/chess/uci/

    SearchConstraints constraints{};
    UciTimeControl utc{};

    // Defaults: no explicit constraints.
    constraints.mode = SearchType::Normal;
    constraints.movetime_ms = -1;
    constraints.depth = -1;
    constraints.nodes = -1;

    // Track whether any time-related token was provided.
    bool saw_any_time_token = false;

    std::istringstream iss(LINE);
    std::string token;
    iss >> token; // "go"

    while (iss >> token)
    {
        if (token == "infinite")
            constraints.mode = SearchType::Infinite;

        else if (token == "ponder")
            constraints.mode = SearchType::Ponder;

        else if (token == "depth")
        {
            iss >> constraints.depth;
            constraints.mode = SearchType::FixedDepth;
        }
        else if (token == "nodes")
        {
            iss >> constraints.nodes;
            constraints.mode = SearchType::NodeLimit;
        }
        else if (token == "movetime")
        {
            iss >> constraints.movetime_ms;
            constraints.mode = (constraints.mode == SearchType::Ponder) ? SearchType::Ponder
                                                                        : SearchType::FixedTime;
        }
        else if (token == "wtime")
        {
            iss >> utc.wtime;
            saw_any_time_token = true;
        }
        else if (token == "btime")
        {
            iss >> utc.btime;
            saw_any_time_token = true;
        }
        else if (token == "winc")
        {
            iss >> utc.winc;
            saw_any_time_token = true;
        }
        else if (token == "binc")
        {
            iss >> utc.binc;
            saw_any_time_token = true;
        }
        else if (token == "movestogo")
        {
            iss >> utc.movestogo;
            saw_any_time_token = true;
        }
        else
        {
            // Other UCI go-parameters are currently ignored:
            // - searchmoves, mate
            // - wtime/btime already handled
            // - etc.
        }
    }

    // If a tournament clock was provided, compute a per-move budget.
    // (FixedTime/FixedDepth/NodeLimit/Infinite/Ponder do not require tc.)
    if (constraints.mode == SearchType::Normal && saw_any_time_token)
        constraints.budget = search::time::TimeManager::compute_budget(board->player, utc);

    // Stop previous search if exists.
    stop_search_worker();

    // Start search and set ponder if needed.
    {
        std::lock_guard<std::mutex> lk(search_mutex);
        search_running = true;

        ponder_active = (constraints.mode == SearchType::Ponder);
        ponder_best = Move{};
    }

    Board board_copy = *board;
    SearchConstraints constraints_copy = constraints;

    search_thread = std::thread([this, board_copy, constraints_copy]() mutable {
        const Move best = chessBot.think(board_copy, constraints_copy);

        // If Ponder, dont print and save the best move.
        if (constraints_copy.mode == SearchType::Ponder)
        {
            std::lock_guard<std::mutex> lk(search_mutex);
            ponder_best = best;
            search_running = false;
            return;
        }

        std::cout << "bestmove " << best.to_string() << std::endl;

        std::lock_guard<std::mutex> lk(search_mutex);
        search_running = false;
    });
}

void ChessGame::parser_parse_uci(const std::string& LINE)
{
    if (LINE == "uci")
    {
        std::cout << "id name Helix" << std::endl;
        std::cout << "id author Marvin Becker" << std::endl;
        std::cout << "uciok" << std::endl;
    }
    if (LINE == "isready")
    {
        std::cout << "readyok" << std::endl;
    }
    if (LINE == "stop")
    {
        stop_search_worker();
        {
            std::lock_guard<std::mutex> lk(search_mutex);
            ponder_active = false;
            ponder_best = Move{};
        }

        return;
    }
    if (LINE == "ponderhit")
    {
        // Convert ponder search into a normal move output: stop the worker and
        // output the best move found during pondering.
        {
            std::lock_guard<std::mutex> lk(search_mutex);
            if (!ponder_active)
                return;
        }

        stop_search_worker();

        Move best{};
        {
            std::lock_guard<std::mutex> lk(search_mutex);
            best = ponder_best;
            ponder_active = false;
            ponder_best = Move{};
        }

        // Could be null if ponderhit is to fast.
        if (best.is_null())
            best = moveGenUtils::get_legal_fallback_move(*board);

        std::cout << "bestmove " << best.to_string() << std::endl;
        return;
    }
    if (LINE == "ucinewgame")
    {
        stop_search_worker();
        board->reset();
        chessBot.reset_tt();
        return;
    }
    if (LINE.rfind("position ", 0) == 0)
    {
        stop_search_worker();

        {
            std::lock_guard<std::mutex> lk(search_mutex);
            ponder_active = false;
            ponder_best = Move{};
        }

        parser_uci_handle_position(LINE);
        return;
    }
    if (LINE.rfind("go", 0) == 0)
    {
        parser_uci_handle_go(LINE);
    }
    if (LINE == "quit")
    {
        stop_search_worker();
        exit(0);
    }
}

void ChessGame::parser_parse_classic(const std::string& LINE)
{
    if (LINE[0] == 'F')
    {
        // Read in FEN notation.
        board->read_fen(LINE.substr(1, LINE.length()));
        board->print_current_board();
        return;
    }
    if (LINE[0] == 'f')
    {
        // Get the FEN.
        std::cout << "Your FEN: " << board->get_fen() << std::endl;
        return;
    }

    // undo the last two moves. (Bot did also move that's why)
    if (LINE == "undo")
    {
        board->pop_last_move();
        board->pop_last_move();
        board->print_current_board();
        return;
    }

    // Check if input has the correct length.
    if (LINE.length() < 4)
    {
        std::cout << "invalid" << std::endl;
        return;
    }

    // Parse the move.

    if (const Move PLAYER_MOVE = board->parse_move(LINE); board->try_to_move_piece(PLAYER_MOVE))
    {
        // Make the move.
        if (board->is_check_mate(board->player == WHITE))
        {
            // Check the opponent for check mate.
            std::cout << "CHECK MATE!" << std::endl;
            return;
        }

        constexpr auto LIMIT =
            SearchConstraints{SearchType::Normal, -1, -1, -1, {1800, 2000, 2000}};

        // Bot can only move legal so no need to check if the move is legal.
        // Check if opponent is in check mate after bots turn.
        const Move MOVE = chessBot.think(*board, LIMIT);
        board->make_move(MOVE);
        board->print_current_board();

        if (board->is_check_mate(board->player == WHITE))
        {
            // Check the opponent for check mate.
            board->print_current_board();
            std::cout << "CHECK MATE!" << std::endl;
        }
    }
    else
    {
        std::cout << "invalid" << std::endl;
    }
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
            board->print_current_board();
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
