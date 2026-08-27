#!/usr/bin/env python3
"""Convert PGN games to the games format of the tree-reuse experiment.

Output: one game per line as space separated UCI moves from the start
position. Games with a custom start position (FEN tag) are skipped, the
bench always starts from the normal initial position.

Usage:
    pip install python-chess
    python3 scripts/pgn_to_uci.py match.pgn [--max-games 30] > tests/data/thesis-games.txt
"""

import argparse
import sys

import chess.pgn


def main():
    parser = argparse.ArgumentParser(description="PGN -> UCI move lists for helix-bench.")
    parser.add_argument("pgn", help="input PGN file")
    parser.add_argument("--max-games", type=int, default=0, help="limit (0 = all games)")
    args = parser.parse_args()

    count = 0
    with open(args.pgn) as f:
        while True:
            game = chess.pgn.read_game(f)
            if game is None:
                break

            # The bench walks games from startpos, skip everything else.
            if game.headers.get("FEN"):
                continue

            moves = [move.uci() for move in game.mainline_moves()]
            if not moves:
                continue

            print(" ".join(moves))
            count += 1
            if args.max_games and count >= args.max_games:
                break

    print(f"{count} games written", file=sys.stderr)


if __name__ == "__main__":
    main()
