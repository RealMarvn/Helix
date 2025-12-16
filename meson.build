project(
    'chess_engine',
    'cpp',
    version : '0.1.0',
    default_options : [
        'cpp_std=c++17',
        'buildtype=debug',
        'warning_level=3'
    ]
)

is_debug = get_option('buildtype') == 'debug'

# --------------------------------------------------------------------
# Engine Library
# --------------------------------------------------------------------

engine_sources = files(
    'engine/main.cpp',
    'engine/core/board.cpp',
    'engine/core/chess_game.cpp',
    'engine/eval/eval.cpp',
    'engine/movement/move_gen.cpp',
    'engine/search/chess_bot.cpp',
    'engine/search/tt.cpp',
)

engine_headers = files(
    'engine/core/board.h',
    'engine/core/board_settings.h',
    'engine/core/chess_game.h',
    'engine/core/move.h',
    'engine/core/piece.h',
    'engine/core/player.h',
    'engine/core/zobrist.h',
    'engine/eval/eval.h',
    'engine/eval/psqt_tables.h',
    'engine/exceptions/fen_exception.h',
    'engine/exceptions/board_exception.h',
    'engine/movement/move_gen.h',
    'engine/search/chess_bot.h',
    'engine/search/tt.h',
    'engine/utils.h',
)

engine_inc = include_directories('engine')

chess_engine_lib = static_library(
    'chess_engine',
    engine_sources + engine_headers,
    include_directories : engine_inc
)

# --------------------------------------------------------------------
# Executable: helix
# --------------------------------------------------------------------

helix_exe = executable(
    'helix',
    'engine/main.cpp',
    include_directories : engine_inc,
    link_with : chess_engine_lib
)

run_target(
    'run',
    command : [helix_exe],
    depends : helix_exe
)

# --------------------------------------------------------------------
# GTest über Subproject (WrapDB)
# --------------------------------------------------------------------

gtest_dep = dependency(
    'gtest',
    required : true,
    fallback : ['gtest', 'gtest_dep']
)

# --------------------------------------------------------------------
# Tests (nur in Nicht-Debug-Builds)
# --------------------------------------------------------------------

if not is_debug
    # Pfad zu tests/data im Source-Tree
    test_data_dir = meson.current_source_dir() / 'tests' / 'data'
    test_data_src = test_data_dir

    # Kopiert tests/data -> build/.../data
    copy_test_data = custom_target(
        'copy_test_data',
        output : 'test_data_copied.stamp',
        command : [
            'bash', '-c',
            'rm -rf "$1" && cp -r "$0" "$1" && touch "$2"',
            test_data_src,
            meson.current_build_dir() / 'data',
            '@OUTPUT@'
        ],
        build_by_default : true
    )

    tests_exe = executable(
        'chess-tests',
        'tests/tests.cpp',
        include_directories : engine_inc,
        link_with : chess_engine_lib,
        dependencies : [gtest_dep],
        cpp_args : ['-DTEST_DATA_DIR="@0@"'.format(test_data_dir)],
        build_by_default : true
    )

    test(
        'chess-tests',
        tests_exe,
        depends : copy_test_data
    )
endif

# --------------------------------------------------------------------
# clang-format / clang-tidy Targets
# --------------------------------------------------------------------

all_source_files = files(
    'engine/main.cpp',
    'engine/core/board.cpp',
    'engine/core/chess_game.cpp',
    'engine/eval/eval.cpp',
    'engine/movement/move_gen.cpp',
    'engine/search/chess_bot.cpp',
    'engine/search/tt.cpp',
    'tests/tests.cpp',
)

clang_format = find_program('clang-format', required : false)
if clang_format.found()
    run_target(
        'format',
        command : [clang_format, '-i'] + all_source_files
    )

    run_target(
        'format-check',
        command : [clang_format, '--dry-run', '--Werror'] + all_source_files
    )
endif

clang_tidy = find_program('clang-tidy', required : false)
if clang_tidy.found()
    run_target(
        'tidy',
        command : [
            clang_tidy,
            '-p', meson.current_build_dir()
        ] + all_source_files
    )
endif