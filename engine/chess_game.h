//
// Created by Marvin Becker on 16.03.24.
//

#pragma once

#include <memory>

#include "./chess_bot.h"

class ChessGame {
public:
  /**
   * @class ChessGame
   * Represents a chess game.
   */
  ChessGame() : board{new Board} {
  };

  /**
   * @brief Starts the chess game and handles the input and gameplay.
   *
   * This function starts the chess game and initializes the parser to read the input.
   * The game continues until a checkmate occurs.
   */
  void start() const;

private:
  std::unique_ptr<Board> board;
  ChessBot chessBot;

  /**
   * @brief Initializes the parser
   *
   * This function continuously reads input from the user.
   * It supports several commands and performs the corresponding actions based on the input.
   * The parser continues until a checkmate occurs.
   */
  void parser_init() const;

  void parser_parse_uci(const std::string& line) const;

  void parser_parse_classic(const std::string& line) const;

  void parser_uci_handle_position(const std::string& line) const;

  void parser_uci_handle_go(const std::string& line) const;
};
