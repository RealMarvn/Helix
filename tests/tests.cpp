//
// Created by Marvin Becker on 13.03.24.
//

#include <gtest/gtest.h>

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "../engine/chess_bot.h"

static std::string data_path(const std::string& filename) { return std::string(TEST_DATA_DIR) + "/" + filename; }

uint64_t perft(Board& boardManager, const int DEPTH, const bool PLAYER) {
  if (DEPTH == 0) {
    return 1;
  }

  uint64_t nodes = 0;
  // Get all moves.
  auto moves = moveGenUtils::get_all_pseudo_legal_moves(boardManager, PLAYER);
  for (Move& move : moves) {
    // If move is valid get the value.
    if (boardManager.make_move(move)) {
      nodes += perft(boardManager, DEPTH - 1, !PLAYER);
      boardManager.pop_last_move();
    }
  }

  return nodes;
}

TEST(MoveGenTest, PerftTest) {
  // Get Perft file!
  std::ifstream epd_file(data_path("perft-positions.epd"));
  ASSERT_TRUE(epd_file.good()) << "The path of the testing suite is wrong. Please change!";
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
    myBoard.read_fen(settings[0]);

    // Check FEN!
    auto result = perft(myBoard, 4, myBoard.player == WHITE);
    ASSERT_EQ(result, std::stoi(settings[4].substr(3)));
  }
}

TEST(Board, GenerateFenTest) {
  // Get Perft file!
  std::ifstream epd_file(data_path("perft-positions.epd"));
  ASSERT_TRUE(epd_file.good()) << "The path of the testing suite is wrong. Please change!";
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
    myBoard.read_fen(settings[0]);

    // Add the space after the fen because the data has one in the end.
    ASSERT_EQ(myBoard.get_fen() + " ", settings[0]);
  }
}

TEST(UserInput, MoveParsing) {
  // Get move file!
  std::ifstream epd_file(data_path("input-test.epd"));
  ASSERT_TRUE(epd_file.good()) << "The path of the testing suite is wrong. Please change!";
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
    myBoard.read_fen(settings[0]);

    // Build the move based on the input string.
    Move move = myBoard.parse_move(settings[1]);
    // Convert the move back to a string and check if it is the same.
    ASSERT_EQ(move.to_string(), settings[1]);
  }
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
