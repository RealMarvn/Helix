cmake_minimum_required(VERSION 3.27)
project(chess-engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

enable_testing()

add_executable(chess-engine engine/main.cpp
        engine/board.cpp
        engine/board.h
        engine/movement/move_gen.cpp
        engine/movement/move_gen.h
        engine/misc/move.h
        engine/misc/piece.h
        engine/chess_bot.cpp
        engine/chess_bot.h
        engine/misc/board_settings.h
        engine/chess_game.cpp
        engine/chess_game.h
        engine/misc/player.h
        engine/exceptions/fen_exception.h
        engine/exceptions/board_exception.h
        engine/misc/zobrist.h
)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Add your test source file
add_executable(tests tests/tests.cpp
        engine/board.cpp
        engine/board.h
        engine/movement/move_gen.cpp
        engine/movement/move_gen.h
        engine/misc/move.h
        engine/misc/piece.h
        engine/chess_bot.cpp
        engine/chess_bot.h
        engine/misc/board_settings.h
        engine/chess_game.cpp
        engine/chess_game.h
        engine/misc/player.h
        engine/exceptions/fen_exception.h
        engine/exceptions/board_exception.h
)

# Link the Google libraries to your test executable
target_link_libraries(tests GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(tests)
