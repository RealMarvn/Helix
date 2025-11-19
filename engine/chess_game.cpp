//
// Created by Marvin Becker on 16.03.24.
//

#include <iostream>

#include "./chess_game.h"

void ChessGame::start() const {
    // First print the board.
    parser_init();
}

void ChessGame::handlePositionCommand(const std::string& line) const {
    std::string token;
    std::istringstream iss(line);

    iss >> token; // "position"
    iss >> token; // "startpos" oder "fen"

    if (token == "startpos") {
        board->reset();

        // Prüfe, ob moves folgen
        if (iss >> token && token == "moves") {
            while (iss >> token) {
                Move m = board->parseMove(token);
                board->tryToMovePiece(m);
            }
        }
    } else if (token == "fen") {
        std::string fen, part;
        while (iss >> part && part != "moves") {
            fen += part + " ";
        }
        board->readFen(fen);

        if (part == "moves") {
            while (iss >> token) {
                Move m = board->parseMove(token);
                board->tryToMovePiece(m);
            }
        }
    }
}

void ChessGame::parser_parse_uci(const std::string& line) const {
    if (line == "uci") {
        std::cout << "id name Helix" << std::endl;
        std::cout << "id author Marvin Becker" << std::endl;
        std::cout << "uciok" << std::endl;
    }
    if (line == "isready") {
        std::cout << "readyok" << std::endl;
    }
    if (line == "ucinewgame") {
        board->reset();
    }
    if (line.rfind("position ", 0) == 0) {
        handlePositionCommand(line);
    }
    if (line.rfind("go", 0) == 0) {
        const Move best = ChessBot::generateBestNextMove(*board);
        std::cout << "bestmove " << best.to_uci_string() << std::endl;
    }
    if (line == "quit") {
        exit(0);
    }
}

void ChessGame::parser_parse_classic(const std::string& line) const {
    if (line[0] == 'F') {
        // Read in FEN notation.
        board->readFen(line.substr(1, line.length()));
        board->printCurrentBoard();
        return;
    }
    if (line[0] == 'f') {
        // Get the FEN.
        std::cout << "Your FEN: " << board->getFen() << std::endl;
        return;
    }

    // undo the last two moves. (Bot did also move that's why)
    if (line == "undo") {
        board->popLastMove();
        board->popLastMove();
        board->printCurrentBoard();
        return;
    }

    // Check if input has the correct length.
    if (line.length() < 4) {
        std::cout << "invalid" << std::endl;
        return;
    }

    // Parse the move.
    Move playerMove = board->parseMove(line);

    if (board->tryToMovePiece(playerMove)) {
        // Make the move.
        if (board->isCheckMate(board->player == WHITE)) {
            // Check the opponent for check mate.
            std::cout << "CHECK MATE!" << std::endl;
            return;
        }

        // Bot can only move legal so no need to check if the move is legal.
        // Check if opponent is in check mate after bots turn.
        Move move = ChessBot::generateBestNextMove(*board);
        board->makeMove(move);
        board->printCurrentBoard();

        if (board->isCheckMate(board->player == WHITE)) {
            // Check the opponent for check mate.
            board->printCurrentBoard();
            std::cout << "CHECK MATE!" << std::endl;
        }
    } else {
        std::cout << "invalid" << std::endl;
    }
}

void ChessGame::parser_init() const {
    bool uci_mode = false;

    // Loop through the input.
    std::string input;
    while (getline(std::cin, input)) {
        // If uci parser does accept string, continue
        if (input == "uci")
            uci_mode = true;
        else if (input == "classic") {
            uci_mode = false;
            board->printCurrentBoard();
            continue;
        }


        // If uci mode is activated
        if (uci_mode) {
            parser_parse_uci(input);
            continue;
        }

        parser_parse_classic(input);
    }
}
