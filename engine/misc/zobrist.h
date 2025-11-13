//
// Created by Marvin Becker on 24.03.24.
//

#pragma once

#include <random>

class Zobrist {
public:
  uint64_t zobrist_squares[12][64]{0};
  uint64_t zobrist_stm[2]{0};
  uint64_t zobrist_ep[8]{0};
  uint64_t zobrist_castling[4]{0};

  Zobrist() {
    for (auto& piece: zobrist_squares) {
      for (unsigned long long& square: piece) {
        square = generate_random_64_bit_number();
      }
    }

    zobrist_stm[0] = generate_random_64_bit_number();
    zobrist_stm[1] = generate_random_64_bit_number();

    for (auto& ep: zobrist_ep) {
      ep = generate_random_64_bit_number();
    }

    for (auto& castling: zobrist_castling) {
      castling = generate_random_64_bit_number();
    }
  }

private:
  static uint64_t generate_random_64_bit_number() {
    std::random_device rd;
    std::mt19937_64 e2(rd());
    std::uniform_int_distribution<uint64_t> dist(0, std::numeric_limits<uint64_t>::max());

    return dist(e2);
  }
};
