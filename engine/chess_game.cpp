//
// Created by Marvin Becker on 16.03.24.
//

#include <iostream>

#include "./chess_game.h"

void ChessGame::start() const {
    // First print the board.
    board->printCurrentBoard();

    // Loop through the input.
    std::string input;
    while (getline(std::cin, input)) {
        if (input[0] == 'F') {
            // Read in FEN notation.
            board->readFen(input.substr(1, input.length()));
            board->printCurrentBoard();
            continue;
        }
        if (input[0] == 'f') {
            // Get the FEN.
            std::cout << "Your FEN: " << board->getFen() << std::endl;
            continue;
        }

        // undo the last two moves. (Bot did also move that's why)
        if (input == "undo") {
            board->popLastMove();
            board->popLastMove();
            board->printCurrentBoard();
            continue;
        }

        // Check if input has the correct length.
        if (input.length() < 5) {
            std::cout << "invalid" << std::endl;
            continue;
        }

        // Parse the move.
        Move playerMove = board->parseMove(input);

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
                return;
            }
        } else {
            std::cout << "invalid" << std::endl;
        }
    }
}
