//
// Created by Marvin Becker on 13.03.24.
//

#include <gtest/gtest.h>

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "../engine/search/search.h"

static std::string data_path(const std::string& filename)
{
    return std::string(TEST_DATA_DIR) + "/" + filename;
}

uint64_t perft(Board& boardManager, const int DEPTH, const bool PLAYER)
{
    if (DEPTH == 0)
    {
        return 1;
    }

    uint64_t nodes = 0;
    // Get all moves.
    auto moves = moveGenUtils::get_pseudo_legal_moves(boardManager, PLAYER);
    for (Move& move : moves)
    {
        // If move is valid get the value.
        if (boardManager.make_move(move))
        {
            nodes += perft(boardManager, DEPTH - 1, !PLAYER);
            boardManager.pop_last_move();
        }
    }

    return nodes;
}

TEST(MoveGenTest, PerftTest)
{
    // Get Perft file!
    std::ifstream epd_file(data_path("perft-positions.epd"));
    ASSERT_TRUE(epd_file.good()) << "The path of the testing suite is wrong. Please change!";
    Board myBoard;

    std::string line;
    // Read in perft file per line.
    while (std::getline(epd_file, line))
    {
        std::istringstream ss(line);
        std::string setting;
        std::vector<std::string> settings;

        // Cut the line into sections.
        while (std::getline(ss, setting, ';'))
        {
            settings.push_back(setting);
        }

        // Read in FEN.
        myBoard.read_fen(settings[0]);

        // Check FEN!
        auto result = perft(myBoard, 4, myBoard.player_ == WHITE);
        ASSERT_EQ(result, std::stoi(settings[4].substr(3)));
    }
}

TEST(Board, GenerateFenTest)
{
    // Get Perft file!
    std::ifstream epd_file(data_path("perft-positions.epd"));
    ASSERT_TRUE(epd_file.good()) << "The path of the testing suite is wrong. Please change!";
    Board myBoard;

    std::string line;
    // Read in perft file per line.
    while (std::getline(epd_file, line))
    {
        std::istringstream ss(line);
        std::string setting;
        std::vector<std::string> settings;

        // Cut the line into sections.
        while (std::getline(ss, setting, ';'))
        {
            settings.push_back(setting);
        }

        // Read in FEN.
        myBoard.read_fen(settings[0]);

        // Add the space after the fen because the data has one in the end.
        ASSERT_EQ(myBoard.get_fen() + " ", settings[0]);
    }
}

TEST(UserInput, MoveParsing)
{
    // Get move file!
    std::ifstream epd_file(data_path("input-test.epd"));
    ASSERT_TRUE(epd_file.good()) << "The path of the testing suite is wrong. Please change!";
    Board myBoard;

    std::string line;
    // Read in perft file per line.
    while (std::getline(epd_file, line))
    {
        std::istringstream ss(line);
        std::string setting;
        std::vector<std::string> settings;

        // Cut the line into sections.
        while (std::getline(ss, setting, ';'))
        {
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

TEST(Board, NullMoveMakeUnmake)
{
    // Get Perft file!
    std::ifstream epd_file(data_path("perft-positions.epd"));
    ASSERT_TRUE(epd_file.good()) << "The path of the testing suite is wrong. Please change!";
    Board myBoard;

    std::string line;
    // Read in perft file per line.
    while (std::getline(epd_file, line))
    {
        std::istringstream ss(line);
        std::string setting;
        std::vector<std::string> settings;

        // Cut the line into sections.
        while (std::getline(ss, setting, ';'))
        {
            settings.push_back(setting);
        }

        // Read in FEN.
        myBoard.read_fen(settings[0]);

        // Remember the state before the null move.
        const auto FEN_BEFORE = myBoard.get_fen();
        const auto HASH_BEFORE = myBoard.get_hash();
        const auto PLAYER_BEFORE = myBoard.player_;

        // A null move must flip the side to move and change the hash.
        myBoard.make_null_move();
        ASSERT_NE(myBoard.player_, PLAYER_BEFORE);
        ASSERT_NE(myBoard.get_hash(), HASH_BEFORE);

        // Popping it must restore the position exactly.
        myBoard.pop_null_move();
        ASSERT_EQ(myBoard.get_fen(), FEN_BEFORE);
        ASSERT_EQ(myBoard.get_hash(), HASH_BEFORE);
        ASSERT_EQ(myBoard.player_, PLAYER_BEFORE);
    }
}

TEST(Board, MakeUnmakeHashRestore)
{
    // Get Perft file!
    std::ifstream epd_file(data_path("perft-positions.epd"));
    ASSERT_TRUE(epd_file.good()) << "The path of the testing suite is wrong. Please change!";
    Board myBoard;

    std::string line;
    // Read in perft file per line.
    while (std::getline(epd_file, line))
    {
        std::istringstream ss(line);
        std::string setting;
        std::vector<std::string> settings;

        // Cut the line into sections.
        while (std::getline(ss, setting, ';'))
        {
            settings.push_back(setting);
        }

        // Read in FEN.
        myBoard.read_fen(settings[0]);
        const auto HASH_BEFORE = myBoard.get_hash();

        // Make every legal move once and check the hash both ways.
        for (Move& move : moveGenUtils::get_pseudo_legal_moves(myBoard, myBoard.player_ == WHITE))
        {
            if (!myBoard.make_move(move))
                continue;

            // The hash after the move must match one built fresh from the FEN.
            Board freshBoard;
            freshBoard.read_fen(myBoard.get_fen());
            ASSERT_EQ(myBoard.get_hash(), freshBoard.get_hash());

            // And popping must restore the hash exactly (no recomputation).
            myBoard.pop_last_move();
            ASSERT_EQ(myBoard.get_hash(), HASH_BEFORE);
        }
    }
}

TEST(Board, HasNonPawnMaterial)
{
    Board myBoard;

    // Start position: both sides have full material.
    ASSERT_TRUE(myBoard.has_non_pawn_material(true));
    ASSERT_TRUE(myBoard.has_non_pawn_material(false));

    // Pure pawn endgame: nobody has non-pawn material.
    myBoard.read_fen("4k3/pppp4/8/8/8/8/4PPPP/4K3 w - - 0 1");
    ASSERT_FALSE(myBoard.has_non_pawn_material(true));
    ASSERT_FALSE(myBoard.has_non_pawn_material(false));

    // Only white keeps a rook.
    myBoard.read_fen("4k3/pppp4/8/8/8/8/4PPPP/R3K3 w - - 0 1");
    ASSERT_TRUE(myBoard.has_non_pawn_material(true));
    ASSERT_FALSE(myBoard.has_non_pawn_material(false));
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
