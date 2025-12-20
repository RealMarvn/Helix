//
// Created by Marvin Becker on 16.03.24.
//

/**
 * @file chess_game.h
 * @brief High-level game controller for running the chess engine.
 *
 * The ChessGame class manages a Board instance, a ChessBot search engine,
 * and user interaction via UCI and classic input modes. It acts as the
 * entry point for engine execution, handling commands, move input, and
 * game flow until termination.
 */

#pragma once

#include <memory>
#include <thread>
#include <mutex>

#include "./search/search.h"

class ChessGame {
public:

    /**
     * @class ChessGame
     * @brief Orchestrates the gameplay, input parsing, and engine interaction.
     *
     * ChessGame owns a Board and a ChessBot instance. It supports two modes:
     *  - UCI mode for GUI/engine communication
     *  - Classic mode for direct human interaction (debug/testing)
     *
     * It continuously processes input, updates the board, and consults the
     * ChessBot to compute engine moves.
     */
    ChessGame() : board_{new Board}, chess_bot_{ChessBot()} {
    }

    /**
     * @brief Starts the chess game and handles the input and gameplay.
     *
     * This function starts the chess game and initializes the parser to read the input.
     * The game continues until a checkmate occurs.
     */
    void start();

private:

    std::thread search_thread_;

    std::mutex search_mutex_;

    /// True while a search worker thread is active.
    bool search_running_ = false;

    /// True if the engine is currently pondering (UCI "go ponder").
    bool ponder_active_ = false;

    /// Best move found during pondering, emitted on "ponderhit".
    Move ponder_best_{};

    /**
     * @brief The current game board.
     *
     * Owns all position-related data including pieces, side to move,
     * castling/en-passant state, and move history.
     */
    std::unique_ptr<Board> board_;

    /**
     * @brief Search engine responsible for selecting best moves.
     *
     * Used whenever the engine must generate a response to a UCI "go"
     * command or when running automated move calculations.
     */
    ChessBot chess_bot_;

    /**
     * @brief Initializes the parser
     *
     * This function continuously reads input from the user.
     * It supports several commands and performs the corresponding actions based on the input.
     * The parser continues until a checkmate occurs.
     */
    void parser_init();

    /**
     * @brief Parses and processes a single input line in UCI mode.
     *
     * Interprets the given line according to the UCI protocol specification.
     * Dispatches the command to the corresponding UCI handler (e.g. "uci", "isready",
     * "position", "go", "quit", etc.).
     *
     * Unknown or malformed commands are safely ignored or reported depending on
     * the parser configuration.
     *
     * @param line The raw input line received from stdin in UCI format.
     */
    void parser_parse_uci(const std::string& line);

    /**
     * @brief Parses and processes a single input line in classic (human) mode.
     *
     * Interprets the given line as a command entered by a human user.
     * Supports commands such as move input, board display, game reset,
     * quitting the application, and debugging commands.
     *
     * This mode is primarily intended for testing and interactive play
     * outside graphical chess GUIs.
     *
     * @param line The raw input line entered by the user.
     */
    void parser_parse_classic(const std::string& line);


    /**
     * @brief Handles the UCI "position" command.
     *
     * Parses and applies a FEN string or "startpos" followed by a sequence
     * of optional moves, and sets up the internal board state accordingly.
     *
     * Supported formats:
     *   - "position startpos"
     *   - "position startpos moves e2e4 e7e5 ..."
     *   - "position fen <FEN string>"
     *   - "position fen <FEN string> moves ..."
     *
     * @param line The full input line containing the "position" command.
     */
    void parser_uci_handle_position(const std::string& line) const;


    /**
     * @brief Handles the UCI "go" command.
     *
     * Parses search-related parameters and starts the engine calculation.
     * May include time controls, depth limits, node limits, or infinite search.
     *
     * Supported parameters include (UCI):
     *  - depth <n>        : Fixed-depth search.
     *  - nodes <n>        : Node-limited search.
     *  - movetime <ms>    : Fixed time per move.
     *  - wtime <ms> btime <ms> winc <ms> binc <ms> [movestogo <n>]
     *                    : Tournament time control; converted into an internal TimeBudget.
     *  - infinite         : Search until a "stop" command is received.
     *  - ponder           : Ponder search; no immediate bestmove output until "ponderhit".
     *
     * @param line The full input line containing the "go" command.
     */
    void parser_uci_handle_go(const std::string& line);

    /**
     * @brief Stops the currently running search worker thread.
     *
     * Requests the active search to stop (if any) and joins the worker thread
     * to ensure a clean and deterministic shutdown of the search.
     *
     * This function is safe to call even if no search is currently running.
     * After completion, no search thread will be active.
     *
     * Note:
     *  - This function does NOT clear ponder-related state.
     *  - Ponder state is managed separately to keep concerns isolated.
     */
    void stop_search_worker();

    /**
     * @brief Clears the internal ponder state (locked variant).
     *
     * Resets all ponder-related flags and stored moves.
     * This function assumes that the caller already holds `search_mutex`.
     *
     * Intended for internal use in performance-critical or compound operations
     * where the mutex is already acquired.
     */
    void clear_ponder_state_locked();

    /**
     * @brief Clears the internal ponder state in a thread-safe manner.
     *
     * Acquires `search_mutex` and resets all ponder-related flags and cached moves.
     * This should be used whenever ponder state needs to be invalidated from
     * outside a locked context (e.g. on "stop", "position", or "ucinewgame").
     */
    void clear_ponder_state();
};
