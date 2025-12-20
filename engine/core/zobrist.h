#pragma once

/**
 * @file zobrist.h
 * @brief Implements Zobrist hashing tables for the chess engine.
 *
 * Zobrist hashing assigns a random 64‑bit number to each possible piece
 * on every square, side-to-move, en-passant file, and castling right.
 * These numbers are XOR‑combined to form a unique hash for any board state,
 * enabling fast repetition detection and transposition table indexing.
 */

#include <random>

/**
 * @brief Holds all Zobrist hash keys used by the engine.
 *
 * Upon construction, all hash components are initialized with uniformly
 * distributed 64‑bit random numbers. The engine uses these keys to compute
 * incremental board hashes during move making and unmaking.
 */
class Zobrist {
public:

    /**
     * @brief Zobrist keys for piece-square combinations.
     *
     * Indexed as [piece_type][square_index], covering all 12 piece types
     * (white/black × pawn/knight/bishop/rook/queen/king) over 64 squares.
     */
    uint64_t zobrist_squares_[12][64]{};

    /**
     * @brief Zobrist keys for side-to-move.
     *
     * Index 0 = white to move, index 1 = black to move.
     */
    uint64_t zobrist_stm_[2]{};

    /**
     * @brief Zobrist keys for en-passant file availability.
     *
     * Only one file may be active at a time. Indexed 0..7 for files a–h.
     */
    uint64_t zobrist_ep_[8]{};

    /**
     * @brief Zobrist keys for castling rights.
     *
     * Four flags: white king-side, white queen-side,
     * black king-side, black queen-side.
     */
    uint64_t zobrist_castling_[4]{};

    /**
     * @brief Initializes all Zobrist keys with random 64‑bit numbers.
     *
     * Called once at engine startup. These values must remain stable
     * throughout the entire engine runtime to ensure consistent hashing.
     */
    Zobrist() {
        for (auto& piece: zobrist_squares_) {
            for (auto& square: piece) {
                square = generate_random_64_bit_number();
            }
        }

        zobrist_stm_[0] = generate_random_64_bit_number();
        zobrist_stm_[1] = generate_random_64_bit_number();

        for (auto& ep: zobrist_ep_) {
            ep = generate_random_64_bit_number();
        }

        for (auto& castling: zobrist_castling_) {
            castling = generate_random_64_bit_number();
        }
    }

private:

    /**
     * @brief Generates a uniformly distributed random 64‑bit number.
     *
     * Used to initialize the Zobrist tables. Relies on std::random_device
     * to seed a Mersenne Twister 64‑bit generator.
     *
     * @return A random 64‑bit unsigned integer.
     */
    static uint64_t generate_random_64_bit_number() {
        std::random_device rd;
        std::mt19937_64 e2(rd());
        std::uniform_int_distribution<uint64_t> dist(0, std::numeric_limits<uint64_t>::max());

        return dist(e2);
    }
};
