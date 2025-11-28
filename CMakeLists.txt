cmake_minimum_required(VERSION 3.27)
project(chess_engine LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Default Build Type if not set: (Debug/Release/RelWithDebInfo/MinSizeRel)
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif ()

message(STATUS "Configuring ${PROJECT_NAME} in ${CMAKE_BUILD_TYPE} mode")

if (MSVC)
    add_compile_options(/W4)
else ()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif ()

# Debug: Sanitizer + -O0 + -g
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    if (NOT MSVC)
        add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -O0 -g)
        add_link_options(-fsanitize=address,undefined)
    endif ()
else ()
    # Release
    if (NOT MSVC)
        add_compile_options(-O3)
    endif ()
endif ()

set(ENGINE_SOURCES
        engine/board.cpp
        engine/movement/move_gen.cpp
        engine/chess_bot.cpp
        engine/chess_game.cpp
)

set(ENGINE_HEADERS
        engine/board.h
        engine/movement/move_gen.h
        engine/misc/move.h
        engine/misc/piece.h
        engine/chess_bot.h
        engine/misc/board_settings.h
        engine/chess_game.h
        engine/misc/player.h
        engine/exceptions/fen_exception.h
        engine/exceptions/board_exception.h
        engine/misc/zobrist.h
        engine/utils.h
)

add_library(chess_engine STATIC
        ${ENGINE_SOURCES}
        ${ENGINE_HEADERS}
)

target_include_directories(chess_engine PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/engine
)

add_executable(helix
        engine/main.cpp
)

target_link_libraries(helix PRIVATE chess_engine)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(chess-tests
        tests/tests.cpp
)

target_link_libraries(chess-tests PRIVATE
        chess_engine
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(chess-tests)

set(ALL_SOURCE_FILES
        ${CMAKE_SOURCE_DIR}/engine/main.cpp
        ${CMAKE_SOURCE_DIR}/engine/board.cpp
        ${CMAKE_SOURCE_DIR}/engine/movement/move_gen.cpp
        ${CMAKE_SOURCE_DIR}/engine/chess_bot.cpp
        ${CMAKE_SOURCE_DIR}/engine/chess_game.cpp
        ${CMAKE_SOURCE_DIR}/tests/tests.cpp
)

# clang-format
find_program(CLANG_FORMAT_EXE NAMES clang-format)

if (CLANG_FORMAT_EXE)
    message(STATUS "clang-format found: ${CLANG_FORMAT_EXE}")

    # cmake --build . --target format
    add_custom_target(format
            COMMAND ${CLANG_FORMAT_EXE} -i ${ALL_SOURCE_FILES}
            COMMENT "Running clang-format on all sources"
    )

    add_custom_target(format-check
            COMMAND ${CLANG_FORMAT_EXE} --dry-run --Werror ${ALL_SOURCE_FILES}
            COMMENT "Checking formatting with clang-format"
    )
else ()
    message(STATUS "clang-format not found – format/format-check targets will be no-ops")
    add_custom_target(format
            COMMAND ${CMAKE_COMMAND} -E echo "clang-format not found"
    )
    add_custom_target(format-check
            COMMAND ${CMAKE_COMMAND} -E echo "clang-format not found"
    )
endif ()

# clang-tidy
find_program(CLANG_TIDY_EXE NAMES clang-tidy)

if (CLANG_TIDY_EXE)
    message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")

    add_custom_target(tidy
            COMMAND ${CLANG_TIDY_EXE}
            -p ${CMAKE_BINARY_DIR}
            ${ALL_SOURCE_FILES}
            COMMENT "Running clang-tidy on all sources"
    )
else ()
    message(STATUS "clang-tidy not found – tidy target will be a no-op")
    add_custom_target(tidy
            COMMAND ${CMAKE_COMMAND} -E echo "clang-tidy not found"
    )
endif ()

# helix ausführen: cmake --build . --target run
add_custom_target(run
        COMMAND helix
        DEPENDS helix
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Running helix"
)

# Tests ausführen: cmake --build . --target test_all
add_custom_target(test_all
        COMMAND ctest --output-on-failure
        DEPENDS chess-tests
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Running all tests"
)