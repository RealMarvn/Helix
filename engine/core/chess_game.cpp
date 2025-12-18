//
// Created by Marvin Becker on 16.03.24.
//

#include <iostream>
#include <sstream>

#include "../search/time/time_manager.h"
#include "./search/time/search_constraints.h"
#include "./search/time/uci_time_control.h"
#include "chess_game.h"

void ChessGame::start()
{
    parser_init();
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

        // Prüfe, ob moves folgen
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
        {
            constraints.mode = SearchType::Infinite;
        }
        else if (token == "ponder")
        {
            constraints.mode = SearchType::Ponder;
        }
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
            constraints.mode = SearchType::FixedTime;
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

    // Run the search.
    const Move BEST_MOVE = chessBot.think(*board, constraints);
    std::cout << "bestmove " << BEST_MOVE.to_string() << std::endl;
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
    if (LINE == "ucinewgame")
    {
        board->reset();
        chessBot.reset_tt();
    }
    if (LINE.rfind("position ", 0) == 0)
    {
        parser_uci_handle_position(LINE);
    }
    if (LINE.rfind("go", 0) == 0)
    {
        parser_uci_handle_go(LINE);
    }
    if (LINE == "quit")
    {
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

        auto limit = SearchConstraints{SearchType::Normal, -1, -1, -1, {1800, 2000, 2000}};

        // Bot can only move legal so no need to check if the move is legal.
        // Check if opponent is in check mate after bots turn.
        const Move MOVE = chessBot.think(*board, limit);
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
