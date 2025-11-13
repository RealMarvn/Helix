//
// Created by Marvin Becker on 13.03.24.
//

#include <gtest/gtest.h>

#include <fstream>
#include <string>

#include "../engine/chess_bot.h"

uint64_t perft(Board& boardManager, int depth, bool player) {
  if (depth == 0) {
    return 1;
  }

  uint64_t nodes = 0;
  // Get all moves.
  auto moves = moveGenUtils::getAllPseudoLegalMoves(boardManager, player);
  for (Move& move : moves) {
    // If move is valid get the value.
    if (boardManager.makeMove(move)) {
      nodes += perft(boardManager, depth - 1, !player);
      boardManager.popLastMove();
    }
  }

  return nodes;
}

TEST(MoveGenTest, PerftTest) {
  // Get Perft file!
  std::ifstream epd_file("./data/perft-positions.epd");
  ASSERT_TRUE(epd_file.good()) << "Der Pfad der testing suite ist falsch. Bitte anpassen!";
  Board myBoard;

  std::string line;
  // Read in perft file per line.
  while (std::getline(epd_file, line)) {
    std::istringstream ss(line);
    std::string setting;
    std::vector<std::string> settings;

    // Cut the line into sections.
    while (std::getline(ss, setting, ';')) {
      settings.push_back(setting);
    }

    // Read in FEN.
    myBoard.readFen(settings[0]);

    // Check FEN!
    auto result = perft(myBoard, 4, myBoard.player == WHITE);
    ASSERT_EQ(result, std::stoi(settings[4].substr(3)));
  }
}

TEST(Board, GenerateFenTest) {
  // Get Perft file!
  std::ifstream epd_file("./data/perft-positions.epd");
  ASSERT_TRUE(epd_file.good()) << "Der Pfad der testing suite ist falsch. Bitte anpassen!";
  Board myBoard;

  std::string line;
  // Read in perft file per line.
  while (std::getline(epd_file, line)) {
    std::istringstream ss(line);
    std::string setting;
    std::vector<std::string> settings;

    // Cut the line into sections.
    while (std::getline(ss, setting, ';')) {
      settings.push_back(setting);
    }

    // Read in FEN.
    myBoard.readFen(settings[0]);

    // Add the space after the fen because the data has one in the end.
    ASSERT_EQ(myBoard.getFen() + " ", settings[0]);
  }
}

TEST(UserInput, MoveParsing) {
  // Get move file!
  std::ifstream epd_file("./data/input-test.epd");
  ASSERT_TRUE(epd_file.good()) << "Der Pfad der testing suite ist falsch. Bitte anpassen!";
  Board myBoard;

  std::string line;
  // Read in perft file per line.
  while (std::getline(epd_file, line)) {
    std::istringstream ss(line);
    std::string setting;
    std::vector<std::string> settings;

    // Cut the line into sections.
    while (std::getline(ss, setting, ';')) {
      settings.push_back(setting);
    }

    // Read in the board.
    myBoard.readFen(settings[0]);

    // Build the move based on the input string.
    Move move = myBoard.parseMove(settings[1]);
    // Convert the move back to a string and check if it is the same.
    ASSERT_EQ(move.toString(), settings[1]);
  }
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
